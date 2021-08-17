// Use OpenGL 3.0+, but don't use GLU
#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <sys/time.h>
}

#include <iostream>
#include <fstream>
#include <string>

std::string const vert_shader_source = 
	"#version 150\n"
	"in vec3 vertex;\n"
	"in vec2 texCoord0;\n"
	"uniform mat4 mvpMatrix;\n"
	"out vec2 texCoord;\n"
	"void main() {\n"
	"	gl_Position = mvpMatrix * vec4(vertex, 1.0);\n"
	"	texCoord = texCoord0;\n"
	"}\n";

std::string const frag_shader_source =
	"#version 150\n"
	"uniform sampler2D frameTex;\n"
	"in vec2 texCoord;\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"	fragColor = texture(frameTex, texCoord);\n"
	"}\n";

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// attribute indices
enum {
	VERTICES = 0,
	TEX_COORDS	
};

// uniform indices
enum {
	MVP_MATRIX = 0,
	FRAME_TEX
};

// app data structure
typedef struct {
	AVFormatContext *fmt_ctx;
	int stream_idx;
	AVStream *video_stream;
	AVCodecContext *codec_ctx;
	AVCodec *decoder;
	AVPacket *packet;
	AVFrame *av_frame;
	AVFrame *gl_frame;
	struct SwsContext *conv_ctx;
	GLuint vao;
	GLuint vert_buf;
	GLuint elem_buf;
	GLuint frame_tex;
	GLuint program;
	GLuint attribs[2];
	GLuint uniforms[2];
} AppData;

// initialize the app data structure
void initializeAppData(AppData *data) {
	data->fmt_ctx = NULL;
	data->stream_idx = -1;
	data->video_stream = NULL;
	data->codec_ctx = NULL;
	data->decoder = NULL;
	data->av_frame = NULL;
	data->gl_frame = NULL;
	data->conv_ctx = NULL;
}

// clean up the app data structure
void clearAppData(AppData *data) {
	if (data->av_frame) av_free(data->av_frame);
	if (data->gl_frame) av_free(data->gl_frame);
	if (data->packet) av_free(data->packet);
	if (data->codec_ctx) avcodec_close(data->codec_ctx);
	if (data->fmt_ctx) avformat_free_context(data->fmt_ctx);
	glDeleteVertexArrays(1, &data->vao);
	glDeleteBuffers(1, &data->vert_buf);
	glDeleteBuffers(1, &data->elem_buf);
	glDeleteTextures(1, &data->frame_tex);
	initializeAppData(data);
}

// read a video frame
bool readFrame(AppData *data) {	
	do {
		if (av_read_frame(data->fmt_ctx, data->packet) < 0) {
			av_free_packet(data->packet);
			return false;
		}
	
		if (data->packet->stream_index == data->stream_idx) {
			int frame_finished = 0;
		
			if (avcodec_decode_video2(data->codec_ctx, data->av_frame, &frame_finished, 
				data->packet) < 0) {
				av_free_packet(data->packet);
				return false;
			}
		
			if (frame_finished) {
				if (!data->conv_ctx) {
					data->conv_ctx = sws_getContext(data->codec_ctx->width, 
						data->codec_ctx->height, data->codec_ctx->pix_fmt, 
						data->codec_ctx->width, data->codec_ctx->height, PIX_FMT_RGB24,
						SWS_BICUBIC, NULL, NULL, NULL);
				}
			
				sws_scale(data->conv_ctx, data->av_frame->data, data->av_frame->linesize, 0, 
					data->codec_ctx->height, data->gl_frame->data, data->gl_frame->linesize);
					
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data->codec_ctx->width, 
					data->codec_ctx->height, GL_RGB, GL_UNSIGNED_BYTE, 
					data->gl_frame->data[0]);
			}
		}
		
		av_free_packet(data->packet);
	} while (data->packet->stream_index != data->stream_idx);
	
	return true;
}

bool buildShader(std::string const &shader_source, GLuint &shader, GLenum type) {
	int size = shader_source.length();
	
	shader = glCreateShader(type);
	char const *c_shader_source = shader_source.c_str();
	glShaderSource(shader, 1, (GLchar const **)&c_shader_source, &size);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		std::cout << "failed to compile shader" << std::endl;
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char *log = new char[length];
		glGetShaderInfoLog(shader, length, &length, log);
		std::cout << log << std::endl;
		delete[] log;
		return false;
	}
	
	return true;
}

// initialize shaders
bool buildProgram(AppData *data) {
	GLuint v_shader, f_shader;
	if (!buildShader(vert_shader_source, v_shader, GL_VERTEX_SHADER)) {
		std::cout << "failed to build vertex shader" << std::endl;
		return false;
	}
	
	if (!buildShader(frag_shader_source, f_shader, GL_FRAGMENT_SHADER)) {
		std::cout << "failed to build fragment shader" << std::endl;
		return false;
	} 
	
	data->program = glCreateProgram();
	glAttachShader(data->program, v_shader);
	glAttachShader(data->program, f_shader);
	glLinkProgram(data->program);
	GLint status;
	glGetProgramiv(data->program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		std::cout << "failed to link program" << std::endl;
		int length;
		glGetProgramiv(data->program, GL_INFO_LOG_LENGTH, &length);
		char *log = new char[length];
		glGetShaderInfoLog(data->program, length, &length, log);
		std::cout << log << std::endl;
		delete[] log;
		return false;
	}
	
	data->uniforms[MVP_MATRIX] = glGetUniformLocation(data->program, "mvpMatrix");
	data->uniforms[FRAME_TEX] = glGetUniformLocation(data->program, "frameTex");
	
	data->attribs[VERTICES] = glGetAttribLocation(data->program, "vertex");
	data->attribs[TEX_COORDS] = glGetAttribLocation(data->program, "texCoord0");
		
	return true;
}

// draw frame in opengl context
void drawFrame(AppData *data) {
	glClear(GL_COLOR_BUFFER_BIT);	
	glBindTexture(GL_TEXTURE_2D, data->frame_tex);
	glBindVertexArray(data->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	glBindVertexArray(0);
	glfwSwapBuffers();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "provide a filename" << std::endl;
		return -1;
	}
	
	// initialize libav
	av_register_all();
	avformat_network_init();
	
	// initialize custom data structure
	AppData data;
	initializeAppData(&data);
	
	// open video
	if (avformat_open_input(&data.fmt_ctx, argv[1], NULL, NULL) < 0) {
		std::cout << "failed to open input" << std::endl;
		clearAppData(&data);
		return -1;
	}
	
	// find stream info
	if (avformat_find_stream_info(data.fmt_ctx, NULL) < 0) {
		std::cout << "failed to get stream info" << std::endl;
		clearAppData(&data);
		return -1;
	}
	
	// dump debug info
	av_dump_format(data.fmt_ctx, 0, argv[1], 0);
	
	// find the video stream
    for (unsigned int i = 0; i < data.fmt_ctx->nb_streams; ++i)
    {
        if (data.fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            data.stream_idx = i;
            break;
        }
    }

    if (data.stream_idx == -1)
    {
		std::cout << "failed to find video stream" << std::endl;
		clearAppData(&data);
		return -1;
    }

    data.video_stream = data.fmt_ctx->streams[data.stream_idx];
    data.codec_ctx = data.video_stream->codec;

	// find the decoder
    data.decoder = avcodec_find_decoder(data.codec_ctx->codec_id);
    if (data.decoder == NULL)
    {
		std::cout << "failed to find decoder" << std::endl;
		clearAppData(&data);
		return -1;
    }

	// open the decoder
    if (avcodec_open2(data.codec_ctx, data.decoder, NULL) < 0)
    {
    	std::cout << "failed to open codec" << std::endl;
        clearAppData(&data);
        return -1;
    }

	// allocate the video frames
    data.av_frame = avcodec_alloc_frame();
    data.gl_frame = avcodec_alloc_frame();
    int size = avpicture_get_size(PIX_FMT_RGB24, data.codec_ctx->width, 
    	data.codec_ctx->height);
    uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
    avpicture_fill((AVPicture *)data.gl_frame, internal_buffer, PIX_FMT_RGB24, 
    	data.codec_ctx->width, data.codec_ctx->height);
	data.packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	// initialize glfw
	if (!glfwInit()) {
		std::cout << "glfw failed to init" << std::endl;
		glfwTerminate();
		clearAppData(&data);
		return -1;
	}
	
	// open a window
	float aspect = (float)data.codec_ctx->width / (float)data.codec_ctx->height;
	int adj_width = aspect * 300;
	int adj_height = 300;
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!glfwOpenWindow(adj_width, adj_height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		std::cout << "failed to open window" << std::endl;
		glfwTerminate();
		clearAppData(&data);
		return -1;
	}
			
	// initialize opengl
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_TEXTURE_2D);
	
	// initialize shaders
	if (!buildProgram(&data)) {
		std::cout << "failed to initialize shaders" << std::endl;
		glfwTerminate();
		clearAppData(&data);
		return -1;
	}
	glUseProgram(data.program);
	
	// initialize renderable
	glGenVertexArrays(1, &data.vao);
	glBindVertexArray(data.vao);
	
	glGenBuffers(1, &data.vert_buf);
	glBindBuffer(GL_ARRAY_BUFFER, data.vert_buf);
	float quad[20] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glVertexAttribPointer(data.attribs[VERTICES], 3, GL_FLOAT, GL_FALSE, 20, 
		BUFFER_OFFSET(0));
	glEnableVertexAttribArray(data.attribs[VERTICES]);
	glVertexAttribPointer(data.attribs[TEX_COORDS], 2, GL_FLOAT, GL_FALSE, 20,
		BUFFER_OFFSET(12));
	glEnableVertexAttribArray(data.attribs[TEX_COORDS]);  
	glGenBuffers(1, &data.elem_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.elem_buf);
	unsigned char elem[6] = {
		0, 1, 2,
		0, 2, 3
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elem), elem, GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &data.frame_tex);
	glBindTexture(GL_TEXTURE_2D, data.frame_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.codec_ctx->width, data.codec_ctx->height, 
		0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glUniform1i(data.uniforms[FRAME_TEX], 0);
	
	glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(data.uniforms[MVP_MATRIX], 1, GL_FALSE, glm::value_ptr(mvp));
		
	bool running = true;

	// run the application mainloop
	while (readFrame(&data) && running) {
		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
		drawFrame(&data);
	}

    avformat_close_input(&data.fmt_ctx);
	
	// clean up
	glfwTerminate();
	clearAppData(&data);
}