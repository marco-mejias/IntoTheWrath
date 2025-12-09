#version 460 core

in vec3 vWorldPos;
in vec3 vNormal;
in float vWave;

out vec4 FragColor;

// --- Obra Dinn / dither ---
uniform bool  uObraDinnOn;
uniform float uThreshold;
uniform float uDitherAmp;
uniform bool  uGammaMap;

// Bayer 8x8
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
    vec3 N = normalize(vNormal);

    // --- Paleta del mar ajustada ---
    // Cap valor és 0.0 → mai negre pur
    const vec3 deepColor  = vec3(0.025, 0.085, 0.070);  // +0.005 / +0.015 / +0.015
    const vec3 midColor   = vec3(0.085, 0.19,  0.145);  // +0.015 / +0.02  / +0.015
    const vec3 crestColor = vec3(0.40, 0.50, 0.45);

    // vWave ≈ [-0.9, 0.9] → [0,1]
    float hNorm = clamp(vWave * 0.6 + 0.5, 0.0, 1.0);  // 0 = vall, 1 = cresta
    float nUp   = clamp(dot(N, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);

    // Zones de transició
    float zoneLow  = smoothstep(0.0, 0.35, hNorm);
    float zoneHigh = smoothstep(0.65, 1.0, hNorm);

    // deep -> mid -> crest, però suau
    vec3 waterColor = mix(deepColor, midColor, zoneLow);
    waterColor      = mix(waterColor, crestColor, zoneHigh * 0.75); // crestes menys heavies

    // Escuma molt suau
    float crest = smoothstep(0.70, 0.98, hNorm) * smoothstep(0.5, 1.0, nUp);
    waterColor  = mix(waterColor, crestColor, crest * 0.4); // abans 0.75, ara més suau

    vec3 baseColor = waterColor;

    // ───────── Sense Obra Dinn ─────────
    if (!uObraDinnOn) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    // ───────── Obra Dinn ─────────
    float luma = dot(baseColor, vec3(0.2126, 0.7152, 0.0722));

    float localThr = uGammaMap ? pow(uThreshold, 2.2) : uThreshold;

    // Ajust suau: crestes lleugerament més "blanques", però sense flipar
    localThr -= crest * 0.15;  // abans -0.25

    float d = bayer8x8(gl_FragCoord.xy) - 0.5;
    localThr = clamp(localThr + d * uDitherAmp, 0.0, 1.0);

    float bw = (luma > localThr) ? 1.0 : 0.0;

    // “Blanc” i “negre” verdosos, però sense negre total ni blanc nuclear
    vec3 darkOD  = deepColor * 0.9;                          // una mica més fosc, però > 0
    vec3 lightOD = mix(midColor, crestColor, 0.7) * 1.05;    // clar, però no crema

    vec3 finalColor = mix(darkOD, lightOD, bw);
    FragColor = vec4(finalColor, 1.0);
}
