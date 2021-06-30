#include "gl_gfx.h"

#include <vector>

void filledCircleColor(const GlGFX& target, int16_t x, int16_t y, int16_t r,
                       uint32_t color) {}

void circleColor(const GlGFX& target, int16_t x, int16_t y, int16_t r,
                 uint32_t color) {}

void filledTrigonColor(const GlGFX& gfx, int16_t ax, int16_t ay, int16_t bx,
                       int16_t by, int16_t cx, int16_t cy, uint32_t color) {
  int8_t r = color & 0xFF;
  int8_t g = (color & 0xFF00) >> 8;
  int8_t b = (color & 0xFF0000) >> 16;
  int8_t a = (color & 0xFF000000) >> 24;
  filledTrigonRGBA(gfx, ax, ay, bx, by, cx, cy, r, g, b, a);
}

void trigonColor(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
                 int16_t by, int16_t cx, int16_t cy, uint32_t color) {}

void lineColor(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
               int16_t by, uint32_t color) {}

void trigonRGBA(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
                int16_t by, int16_t cx, int16_t cy, uint8_t r, uint8_t g,
                uint8_t b, uint8_t a) {}

void filledTrigonRGBA(const GlGFX& gfx, int16_t ax, int16_t ay, int16_t bx,
                      int16_t by, int16_t cx, int16_t cy, uint8_t r, uint8_t g,
                      uint8_t b, uint8_t a) {
  float red, green, blue;
  red = r / 255.f;
  green = g / 255.f;
  blue = b / 255.f;
  gfx.triangles_.push_back({(float)ax, (float)ay, {red, green, blue, 1.0}});
  gfx.triangles_.push_back({(float)bx, (float)by, {red, green, blue, 1.0}});
  gfx.triangles_.push_back({(float)cx, (float)cy, {red, green, blue, 1.0}});
}

namespace {

GLuint common_get_shader_program(const char* vertex_shader_source,
                                 const char* fragment_shader_source) {
  GLchar* log = NULL;
  GLint log_length, success;
  GLuint fragment_shader, program, vertex_shader;

  /* Vertex shader */
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
  std::vector<GLchar> log_message;
  log_message.resize(log_length);
  if (log_length > 0) {
    glGetShaderInfoLog(vertex_shader, log_length, NULL, log_message.data());
    printf("vertex shader log:\n\n%s\n", log_message.data());
  }
  if (!success) {
    printf("vertex shader compile error\n");
    exit(EXIT_FAILURE);
  }

  /* Fragment shader */
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 0) {
    log_message.resize(log_length);
    glGetShaderInfoLog(fragment_shader, log_length, NULL, log_message.data());
    printf("fragment shader log:\n\n%s\n", log_message.data());
  }
  if (!success) {
    printf("fragment shader compile error\n");
    exit(EXIT_FAILURE);
  }

  /* Link shaders */
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 0) {
    log_message.resize(log_length);
    glGetProgramInfoLog(program, log_length, NULL, log_message.data());
    printf("shader link log:\n\n%s\n", log_message.data());
  }
  if (!success) {
    printf("shader link error");
    exit(EXIT_FAILURE);
  }

  /* Cleanup. */
  free(log);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  return program;
}
}  // namespace

GlGFX::GlGFX(int width, int height)
    : width(width)
    , height(height) {
  glewInit();
  static const GLchar* vertex_shader_source =
      "#version 120\n"
      "uniform float width;\n"
      "uniform float height;\n"
      "attribute vec2 coord2d;\n"
      "attribute vec4 color;\n"
      "varying vec4 outColor;\n"
      "void main() {\n"
      "    float x = (coord2d[0] / width * 2) - 1;\n"
      "    float y = -(coord2d[1] / height * 2) + 1;\n"
      "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
      "    outColor = vec4(color[0], color[1], color[2], 0.0);\n"
      "}\n";
  static const GLchar* fragment_shader_source =
      "#version 120\n"
      "varying vec4 outColor;"
      "void main() {\n"
      "    gl_FragColor = outColor;\n"
      "}\n";

  /* Shader setup. */
  program =
      common_get_shader_program(vertex_shader_source, fragment_shader_source);
  uniform_width = glGetUniformLocation(program, "width");
  uniform_height = glGetUniformLocation(program, "height");
  attribute_coord2d = glGetAttribLocation(program, "coord2d");
  attribute_color = glGetAttribLocation(program, "color");

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void GlGFX::deinit() {
  glDeleteProgram(program);
}

void GlGFX::flush() {
  if (!triangles_.empty()) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, triangles_.size() * sizeof(Vertex),
                 triangles_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glVertexAttribPointer(attribute_color, 4, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(2 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(program);

    glUniform1f(uniform_width, width);
    glUniform1f(uniform_height, height);
    glEnableVertexAttribArray(attribute_coord2d);
    glEnableVertexAttribArray(attribute_color);
    glDrawArrays(GL_TRIANGLES, 0, triangles_.size());
    glDisableVertexAttribArray(attribute_coord2d);
    glDisableVertexAttribArray(attribute_color);

    glDeleteBuffers(1, &vbo);
    triangles_.clear();
  }
}

void GL_GFX_Clear() {
  glClear(GL_COLOR_BUFFER_BIT);
}
