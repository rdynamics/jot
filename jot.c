#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t x;
    size_t y;
    int width;
    int height;
    int channels;
    unsigned char* data;
} imagedata;

float convert(size_t is, size_t coord) {
    return (float)coord / is;
}

int main(int argc, char** argv) {
    if(argc < 3) {
        puts("Need at least two arguments: output name, and at least one input image.");
        return 1;
    }
    
    /* Name of the "image" we are saving */
    char* set_name = argv[1];
    
    /* Determine output file names */
    size_t out_name_size = strlen("images_") + strlen(argv[1]) + strlen(".*") + 1;
    char* out_c = malloc(out_name_size * sizeof(char));
    char* out_h = malloc(out_name_size * sizeof(char));
    
    *out_c = *out_h = '\0';
    
    strcat(out_c, "images_");
    strcat(out_c, set_name);
    strcat(out_c, ".c");
    
    strcat(out_h, "images_");
    strcat(out_h, set_name);
    strcat(out_h, ".h");
    
    FILE *out = fopen(out_c, "w");
    if(!out) {
        puts("Could not open output C file.");
        return 2;
    }
    
    FILE *header = fopen(out_h, "w");
    if(!header) {
        puts("Could not open output H file.");
        return 3;
    }
    
    free(out_c);
    free(out_h);
    
    /* Cut off the first two arguments, as all they are is the name of the
     * program and the output name; these arguments aren't needed here on
     * out, and reducing argv / argc to only what is needed makes everything
     * else slightly simpler
     */
    argv += 2;
    argc -= 2;
    
    imagedata* inputs = malloc(sizeof(imagedata) * argc);
    
    /* Total area needed to store all images */
    int space_needed = 0;
    
    size_t i, j;
    
    for(i = 0; i < argc; ++i) {
        imagedata *in = inputs + i;
        in->data = stbi_load(argv[i], &(in->width), &(in->height), &(in->channels), 0);
        space_needed += in->width * in->height;
    }
    
    size_t min = 1;
    size_t actual = 0;
    
    size_t total = 0;
    size_t height = 0;
    
    /* Add up all unique widths of images and find maximum total height of images
     * by unique width.
     */
    for(;;) {
        actual = 0;
        
        for(i = 0; i < argc; ++i) {
            if(inputs[i].width >= min) {
                actual = inputs[i].width;
                break;
            }
        }
        
        if(actual == 0) { break; }
        
        for(i = 0; i < argc; ++i) {
            if(inputs[i].width >= min && inputs[i].width < actual) {
                actual = inputs[i].width;
            }
        }
        
        printf("Got column width %d\n", actual);
        
        total += actual;
        min = actual + 1;
        
        size_t column_height = 0;
        
        for(i = 0; i < argc; ++i) {
            if(inputs[i].width == actual) {
                column_height += inputs[i].height;
            }
        }
        
        if(column_height > height) { height = column_height; }
    }
    
    printf("Required height: %d\n", height);
    
    size_t image_size = 1;
    /* Find smallest power-of-two image size */
    while(image_size < total || image_size < height) image_size *= 2;
    
    printf("Required image size: %d\n", image_size);
    
    /* Image data */
    size_t image_space = image_size * image_size * 4;
    unsigned char* data = malloc(sizeof(unsigned char) * image_space);
    
    size_t x;
    size_t width = 0;
    size_t y;
    
    /* Loop through all texture widths; "min" now specifies largest width */
    for(i = 1; i <= min; ++i) {
        height = 0;
        
        int width_valid = 0;
        
        for(j = 0; j < argc; ++j) {
            if(inputs[j].width == i) {
                width_valid = 1;
                
                (inputs + j)->x = width;
                (inputs + j)->y = height;
                
                for(y = 0; y < inputs[j].height; ++y) {
                    for(x = 0; x < i; ++x) {
                        for(size_t comp = 0; comp < 4; ++comp) {
                            data[4 * ((x + width) + ((y + height) * image_size)) + comp] = inputs[j].data[4 * (x + (y * i)) + comp];
                        }
                    }
                }
                height += inputs[j].height;
                
                puts("Packed image");
            }
        }
        
        if(width_valid) {
            width += i;
            printf("Packed column %d\n", i);
        }
    }
    
    puts("Writing code...");
    fprintf(out, "#include <GLFW/glfw3.h>\n\nGLuint load_image_%s() {\n    unsigned char data[] = { \n", set_name);
    
    fputs("Writing image...", stdout);
    
    for(i = 0; i < image_space; ++i) {
        fprintf(out, "%#x", data[i]);
        
        if(i < image_space - 1) {
            fputs(", ", out);
        }
        
        if(i % 20) fputc('.', stdout);
    }
    
    fputc('\n', stdout);
    
    puts("Wrote image");
    
    puts("Writing code...");
    
    fprintf(out, "\n    }\n    glEnable(GL_TEXTURE_2D);\n    GLuint tex;\n    glGenTextures(1, &tex);\n    glBindTexture(GL_TEXTURE_2D, tex);\n\n    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, %d, %d, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);\n\n    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);\n    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);\n    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);\n    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);\n\n    return tex;\n}", image_size, image_size);
    
    fflush(out);
    
    fprintf(header, "#ifndef IMAGE_INDEX_%s\n#define IMAGE_INDEX_%s\n\n#include <retrodynamics/index>\n\n", set_name, set_name);
    
    puts("Writing index...");
    
    for(i = 0; i < argc; ++i) {        
        char* name = argv[i];
        
        j = 0;
        while(name[j] != '\0') ++j;
        
        while(name[j] != '.') --j;
        name[j] = '\0';
        
        while(name[j] != '/' && name[j] != '\\') {
            --j;
            if(j == 0) break;
        }
        name = name + j;
        
        fprintf(header, "sprite sprite_%s = { %g, %g, %g, %g, %d, %d };\n", name,
            convert(image_size, inputs[i].x),
            convert(image_size, inputs[i].y),
            convert(image_size, inputs[i].x + inputs[i].width),
            convert(image_size, inputs[i].y + inputs[i].height),
            inputs[i].width,
            inputs[i].height
        );
        
        free(inputs[i].data);
    }
    
    fputs("\n#endif", header);
    
    free(data);
    
    fclose(out);
    fclose(header);
}