#version 460 core

in vec3 TexCoords;
uniform samplerCube skybox;
out vec4 FragColor;

// --- Uniforms compartits amb el Phong ---
uniform bool  uObraDinnOn;   // activa el mode Obra Dinn
uniform float uThreshold;    // 0..1
uniform float uDitherAmp;    // 0.0..1.0
uniform bool  uGammaMap;     // pow(thr,2.2) com al Phong

// Mateix Bayer 8x8 que a phong_shdrML.frag
float bayer8x8(vec2 p) {
    int x = int(mod(p.x, 8.0));
    int y = int(mod(p.y, 8.0));
    int M[64] = int[64](
         0,32, 8,40, 2,34,10,42,
        48,16,56,24,50,18,58,26,
        12,44, 4,36,14,46, 6,38,
        60,28,52,20,62,30,54,22,
         3,35,11,43, 1,33, 9,41,
        51,19,59,27,49,17,57,25,
        15,47, 7,39,13,45, 5,37,
        63,31,55,23,61,29,53,21
    );
    return float(M[y*8 + x]) / 64.0;
}

void main()
{    
    vec3 col = texture(skybox, TexCoords).rgb;

    // Si no volem estil Obra Dinn, deixem el skybox normal
    if (!uObraDinnOn) {
        FragColor = vec4(col, 1.0);
        return;
    }

    // --- Mateixa lògica que al Phong per B/N + dither ---
    float luma = dot(col, vec3(0.2126, 0.7152, 0.0722));

    float thr  = uGammaMap ? pow(uThreshold, 2.2) : uThreshold;
    float d    = bayer8x8(gl_FragCoord.xy) - 0.5;
    thr        = clamp(thr + d * uDitherAmp, 0.0, 1.0);

    float bw = (luma > thr) ? 1.0 : 0.0;

    vec3 darkColor  = vec3(0.02, 0.08, 0.05);   // fosc, encara verd però més suau
    vec3 lightColor = vec3(0.70, 0.82, 0.78);   // quasi gris clar amb toc verd


    vec3 finalColor = mix(darkColor, lightColor, bw);

    FragColor = vec4(finalColor, 1.0);
}
