#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 uOffset;

void main()
{
    vUV = aUV;

    // Aplicamos el desplazamiento horizontal/vertical
    vec2 pos = aPos + uOffset;

    gl_Position = vec4(pos, 0.0, 1.0);
}
