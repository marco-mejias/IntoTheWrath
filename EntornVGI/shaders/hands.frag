#version 460 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uHandTex;
uniform int  uCols;
uniform int  uRows;
uniform int  uFrameIndex;
uniform bool  uObraDinnOn;
uniform float uThreshold;
uniform float uDitherAmp;
uniform bool  uGammaMap;

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
    int cols = max(uCols, 1);
    int rows = max(uRows, 1);
    int maxFrames = cols * rows;
    int idx = uFrameIndex;
    if (idx < 0) idx = 0;
    if (idx >= maxFrames) idx = maxFrames - 1;

    int col = idx % cols;
    int row = idx / cols;

    float fCols = float(cols);
    float fRows = float(rows);
    vec2  tileSize = vec2(1.0 / fCols, 1.0 / fRows);
    vec2 offset = vec2(float(col), float(row)) * tileSize;
    vec2 uv     = offset + vUV * tileSize;

    vec4 tex = texture(uHandTex, uv);

    if (tex.a < 0.1)
        discard;

    vec3 colRGB = tex.rgb;
    float alpha = tex.a;

    // 🔦 Factor de oscuridad global para las manos
    // 1.0 = igual que ahora
    // 0.7 = un poco más oscuras
    // 0.5 = bastante más oscuras
    float handDarken = 0.7;

    if (uObraDinnOn) {
        float luma = dot(colRGB, vec3(0.2126, 0.7152, 0.0722));
        float thr  = uGammaMap ? pow(uThreshold, 2.2) : uThreshold;
        float d    = bayer8x8(gl_FragCoord.xy) - 0.5;
        thr        = clamp(thr + d * uDitherAmp, 0.0, 1.0);
        float bw = (luma > thr) ? 1.0 : 0.0;

        vec3 darkColor  = vec3(0.03, 0.10, 0.06);
        vec3 lightColor = vec3(0.96);

        vec3 finalColor = mix(darkColor, lightColor, bw);

        // ⬇️ Las oscurecemos un poco también en Obra Dinn
        finalColor *= handDarken;

        FragColor = vec4(finalColor, alpha);
    }
    else {
        // ⬇️ Aquí simplemente oscurecemos la textura original
        FragColor = vec4(colRGB * handDarken, alpha);
    }
}
