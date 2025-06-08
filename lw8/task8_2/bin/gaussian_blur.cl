inline float4 GammaToLinear(float4 c) {
    return (float4)(pow(c.x, 2.2f), pow(c.y, 2.2f), pow(c.z, 2.2f), c.w);
}

inline float4 LinearToGamma(float4 c) {
    return (float4)(pow(c.x, 1.0f / 2.2f), pow(c.y, 1.0f / 2.2f), pow(c.z, 1.0f / 2.2f), c.w);
}

__kernel void GaussianBlurHorizontal(__global const float4* input_image,
                                     __global float4* output_image,
                                     __global const float* filter_weights,
                                     int filter_radius,
                                     int image_width,
                                     int image_height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= image_width || y >= image_height) return;

    float4 sum = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    for (int k = -filter_radius; k <= filter_radius; ++k) {
        int nx = clamp(x + k, 0, image_width - 1);
        float4 color = input_image[y * image_width + nx];
        color = GammaToLinear(color);
        sum += filter_weights[k + filter_radius] * color;
    }
    output_image[y * image_width + x] = sum;
}

__kernel void GaussianBlurVertical(__global const float4* input_image,
                                   __global float4* output_image,
                                   __global const float* filter_weights,
                                   int filter_radius,
                                   int image_width,
                                   int image_height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= image_width || y >= image_height) return;

    float4 sum = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    for (int k = -filter_radius; k <= filter_radius; ++k) {
        int ny = clamp(y + k, 0, image_height - 1);
        float4 color = input_image[ny * image_width + x]; // Данные уже в линейном пространстве
        sum += filter_weights[k + filter_radius] * color;
    }
    output_image[y * image_width + x] = LinearToGamma(sum);
}