#version 450

layout(local_size_x = 16, local_size_y = 16) in;

layout(std140) uniform uSimulationConfig
{
    float emissionRate;     // [g/s]
    float windSpeed;        // [m/s]
    float effectiveHeight;  // [m]
    float depositionCoeff;  // [1/s]
    // Simulation bounds
    float xMax;             // [m]
    float yMax;             // [m]
    // Grid resolution
    int xRes;               // [1]
    int yRes;               // [1]
    // Atmospheric stability
    float stabilityA;       // [1]
    float stabilityB;       // [1]
};

layout(r32f, binding = 1) uniform image2D uConcentrationImage;

float gaussianConcentration(float x, float y)
{
    if (x <= 0.0)
        return 0.0;

    float sy = stabilityA * x;
    float sz = stabilityB * x;
    float expoY = exp(-(y * y) / (2.0 * sy * sy));
    float expoZ = exp(-(effectiveHeight * effectiveHeight) / (2.0 * sz * sz));
    float base = emissionRate / (2.0 * 3.14159265359 * windSpeed * sy * sz);
    float dep = exp(-depositionCoeff * x / windSpeed);

    return base * expoY * expoZ * dep;
}

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

    if (gid.x >= xRes || gid.y >= yRes)
        return;

    float x = mix(1.0, xMax, float(gid.x) / float(xRes - 1));
    float y = mix(-yMax, yMax, float(gid.y) / float(yRes - 1));

    float c = gaussianConcentration(x, y);

    imageStore(uConcentrationImage, gid, vec4(c, 0.0, 0.0, 1.0));
}