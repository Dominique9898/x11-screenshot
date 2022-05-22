#include <unistd.h>
#include <pwd.h>
#include <vector>
//#include <ctime>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <xcb/xcb.h>
#include <png.h>
// #include <cstdlib>
// #include <stdexcept>
 #include <iostream>
using namespace std;
std::string save_path;
int s_width;
int s_height;
std::vector<std::vector<unsigned char>> image_data;


std::string get_pwd();
std::vector<std::vector<unsigned char>> process_original(XImage * image);
bool save_to_png(std::string path);
void takeScreenShot();

int main() {
    takeScreenShot();
    return 0;
}
std::string get_pwd() {
    time_t now = time(NULL);
    const char* homedir = getpwuid(getuid()) -> pw_dir;
    std::string s_now = std::to_string(now);
    std::string s_homedir = homedir;
    std::string file_path = "/tmp/fullscreen_";
    std::string save_path = file_path + s_now + ".png";

//    std::string file_path = "/tmp/fullscreen_";
//    std::string save_path = s_homedir + file_path + s_now + ".png";
    std::cout << "ScreenShotUtil::get_pwd create save_path=" <<save_path << std::endl;
    return save_path;
}

std::vector<std::vector<unsigned char>> process_original(XImage * image) {
    std::vector<std::vector<unsigned char> > image_data;
    std::vector<unsigned char> image_data_row;
    unsigned long red_mask = image->red_mask;
    unsigned long green_mask = image->green_mask;
    unsigned long blue_mask = image->blue_mask;

    for (int y = 0; y < s_height; y++)
    {
        for (int x = 0; x < s_width; x++)
        {
            unsigned long pixel = XGetPixel(image, x, y);

            unsigned char blue = pixel & blue_mask;
            unsigned char green = (pixel & green_mask) >> 8;
            unsigned char red = (pixel & red_mask) >> 16;

            image_data_row.push_back(red);
            image_data_row.push_back(green);
            image_data_row.push_back(blue);
        }
        image_data.push_back(image_data_row);
        image_data_row.clear();
    }

    return image_data;
};

bool save_to_png(std::string path) {
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row;

    fp = fopen(path.c_str(), "wb");
    if (!fp) {
        return false;
    }

    png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING,
        NULL,
        NULL,
        NULL
    );
    if (!png_ptr) {
        std::cout << "ScreenShotUtil Failed to create PNG write structure" << std::endl;
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        std::cout << "ScreenShotUtil Failed to create PNG info structure" << std::endl;
        fclose(fp);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return false;
    }

    // Setup png lib error handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_init_io(png_ptr, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
        png_ptr,
        info_ptr,
        s_width,
        s_height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    // set compression level
    png_set_compression_level(png_ptr, PNG_Z_DEFAULT_COMPRESSION);
    // write info header
    png_write_info(png_ptr, info_ptr);
    for(std::vector<std::vector<unsigned char>>::size_type i = 0; i != image_data.size(); i++) {
        // build character row from array of characters
        row = (png_bytep) reinterpret_cast<unsigned char*>(image_data[i].data());
        // write byterow
        png_write_row(png_ptr, row);
    }
    // end writing file
    png_write_end(png_ptr, NULL);
    // cleanup
    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

    return true;
};

void takeScreenShot()
{
    save_path = get_pwd();
    Display* display = XOpenDisplay(NULL);

    if (display == NULL) {
        std::cout << "ScreenShotUtil No display can be aquired" << std::endl;
        exit(1);
    }
    
    Window root = DefaultRootWindow(display);

    XWindowAttributes attributes = {0};
    XGetWindowAttributes(display, root, &attributes);

    s_width = attributes.width;
    s_height = attributes.height;

    std::cout << "ScreenShotUtil::takeScreenShot w = " << s_width << " h = " << s_height << std::endl;

    XImage* img = XGetImage(display, root, 0, 0 , s_width, s_height, AllPlanes, ZPixmap);

    image_data = process_original(img);
    if (save_to_png(save_path)) {
        std::cout << "ScreenShotUtil::takeScreenShot Succesfully saved to " << save_path << std::endl;
    } else {
        std::cout << "ScreenShotUtil::takeScreenShot Failed saved to " << save_path << std::endl;
    }

    XDestroyImage(img);
    XCloseDisplay(display);

}
