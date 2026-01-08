#version 450

struct EmitterInfo
{
    vec2 position;
    float emissionRate;
    float height;
};

layout(local_size_x = 16, local_size_y = 16) in;

layout(std140, binding = 1) uniform uSimulationConfig
{
    vec2 size;              // [m]
    vec2 stability;         // [1]
    float windSpeed;        // [m/s]
    float windDir;          // [rad]
    float depositionCoeff;  // [1/s]

    ivec2 resolution;       // [1]
    int emittersCount;
};

layout(std430, binding = 2) readonly buffer uEmitters
{
    EmitterInfo emitters[];
};

layout(r32f, binding = 1) uniform image2D uConcentrationImage;

vec2 rotateToWindFrame(vec2 delta)
{
    float c = cos(windDir);
    float s = sin(windDir);

    return vec2(
        delta.x * c + delta.y * s,
        -s * delta.x + delta.y * c);
}

float gaussianConcentration(EmitterInfo e, vec2 pos)
{
    vec2 posRel = rotateToWindFrame(pos - e.position);

    if (posRel.x <= 0.0)
        return 0.0;

    vec2 stabilityRel = stability * posRel.x;
    float effectiveHeight = e.height;
    float expoY = exp(-(pos.y * pos.y) / (2.0 * stabilityRel.x * stabilityRel.x));
    float expoZ = exp(-(effectiveHeight * effectiveHeight) / (2.0 * stabilityRel.y * stabilityRel.y));
    float base = e.emissionRate / (2.0 * 3.14159265359 * windSpeed * stabilityRel.x * stabilityRel.y);
    float dep = exp(-depositionCoeff * pos.x / windSpeed);

    return base * expoY * expoZ * dep;
}

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

    if (gid.x >= resolution.x || gid.y >= resolution.y)
        return;

    float x = mix(1.0, size.x, float(gid.x) / float(resolution.x - 1));
    float y = mix(-size.y, size.y, float(gid.y) / float(resolution.y - 1));

    float concentration = 0.0;
    for (int i = 0; i < emittersCount; i++)
    {
        concentration += gaussianConcentration(emitters[i], vec2(x, y));
    }

    imageStore(uConcentrationImage, gid, vec4(concentration, 0.0, 0.0, 1.0));
}