#version 130
 
precision highp float; // Video card drivers require this next line to function properly

out vec4 fragColor;
 
void main(void) {
    fragColor = vec4(1.0,1.0,1.0,1.0); //Set colour of each fragment to WHITE
}
