#version 130
 
in  vec3 in_Position;
in  vec2 in_Texcoord;
in  vec3 in_Normal;

uniform struct {
  vec3 position;
  mat4 rotation;
} camera;
 
void main(void) {
  vec3 pos = in_Position - camera.position;
  gl_Position = camera.rotation * vec4(pos.xyz, 1.0);
}
