// DUKE and DUKE2 file formats by NDRAEY  > /\/

#include <iostream>
#include <vector>
#include <png.h>
#include "palette.hpp"

#define DUKE2_MAGIC ("RUKE")

using namespace std;

class Duke {
public:
	
	struct Header {
		char magic[4];
		uint8_t version;
		
		uint16_t width;
		uint16_t height;
		uint8_t alpha;

		uint16_t palette_size;
	} __attribute__((packed));

	// Every palette entry represents RGB (3 byte) or RGBA (4 byte) array.
	// If we use palette mode, we write palette indexes (unsigned short) instead of pixels.

	struct RGBPixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} __attribute__((packed));

	struct RGBAPixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} __attribute__((packed));

	Duke(const char* filename) {
		file = fopen(filename, "w+b");

		if(!file) {
			cout << "Can't open file!!!" << endl;

			exit(1);
		}

		header.version = 0x02;

		#warning Palette mode!
		palette_mode = true;
	}
	
	~Duke() {
		if(imagedata)
			delete[] imagedata;

		if(file)
			fclose(file);
	}

	bool check() {  // Ok
		if(strncmp(header.magic, DUKE2_MAGIC, 4) == 0) {
			return true;
		}

		// TODO: Some other checks

		return false;
	}

	void read_header() {
		fread(&header, 1, sizeof(Header), file);
	}

	void write_header() {
		fwrite(&header, 1, sizeof(Header), file);
	}

	void from_png(char* filename) {
		FILE* png_file = fopen(filename, "rb");
	
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop info = png_create_info_struct(png);
		png_init_io(png, png_file);
		png_read_info(png, info);

		header.width = png_get_image_width(png, info);
		header.height = png_get_image_height(png, info);
		
		int color_type = png_get_color_type(png, info);

		header.alpha = (color_type & PNG_COLOR_MASK_ALPHA) ? 1 : 0;

		png_bytep* row_pointers = new png_bytep[header.height];
		
		for (png_uint_32 y = 0; y < header.height; y++) {
		    row_pointers[y] = new png_byte[png_get_rowbytes(png, info)];
		}
		
		png_read_image(png, row_pointers);

		// HERE

		idata_size = png_get_rowbytes(png, info) * header.height;
		
		cout << "Data size: " << idata_size << endl;
		
		imagedata = new uint8_t[idata_size];

		for (png_uint_32 y = 0; y < header.height; y++) {
			memcpy(
				imagedata + (y * png_get_rowbytes(png, info)),
				row_pointers[y],
				png_get_rowbytes(png, info)
			);
		}

		// OK

		for (png_uint_32 y = 0; y < header.height; y++) {
		    delete[] row_pointers[y];
		}
		
		delete[] row_pointers;
		
		png_destroy_read_struct(&png, &info, NULL);
		
		fclose(png_file);
	}

	void generate_palette() {
		std::cout << "Alpha!" << endl;
		
		palette = buildPalette((const char*)imagedata, header.width, header.height, header.alpha);

		if(palette.size() >= 65535) {
			std::cout << "Palette mode disabled! (Individual pixels > 65535)" << endl;

			palette_mode = false;

			return;
		}

		header.palette_size = palette.size();

		std::cout << "Palette size: "
				  << header.palette_size
				  << " entries"
				  << std::endl;
	}

	void write_palette() {
		auto size = sizeof(Pixel);

		if(!header.alpha)
			size--;

		for(Pixel pixel : palette) {
			fwrite(&pixel, 1, size, file);
		}
	}

	std::size_t find_palette_index(const Pixel& pixel) {
		auto it = std::find_if(
			palette.begin(),
			palette.end(),
			[pixel](const Pixel& px) {
		    	return px == pixel;
			}
		);
		
		if(it == palette.end()) {
			std::cout << "What a? Palette entry was not found!!!" << endl;
			std::cout << "Hey you! Check your file or report me a bug!" << endl;

			exit(1);
		}

		return std::distance(palette.begin(), it);
	}

	void write_pixel_indices() {
		std::cout << "Start!" << endl;

		int step = (header.alpha?4:3);
		std::size_t datasize = idata_size / (header.alpha?4:3);

		for(std::size_t i = 0; i < datasize; i++) {
			auto index = find_palette_index({
				imagedata[(i * step)],
				imagedata[(i * step) + 1],
				imagedata[(i * step) + 2],
				header.alpha ? (char)imagedata[(i * step) + 3] : (char)0xFF
			});

			fwrite(&index, 1, sizeof(short), file);
		}

		std::cout << "Stop!" << endl;
	}

	void save() {
		// Copy our magic.
		memcpy(header.magic, DUKE2_MAGIC, 4);

		// Write our header.
		write_header();

		// Check palette expediency and generate palette.
		generate_palette();

		if(palette_mode) {
			// Write palette indexes instead of RAW pixels
			write_palette();

			std::cout << "Palette written!" << std::endl;

			write_pixel_indices();
		} else {
			// Write raw pixels!
			fwrite(imagedata, 1, idata_size, file);
		}
	}

	friend ostream& operator << (ostream& o, const Duke& img) {
		return (
			o << "DUKE<" << img.header.width << "x" << img.header.height << (img.header.alpha?"; ALPHA":"") << ">"
		);
	}

private:

	FILE* file = nullptr;

	struct Header header;
	
	std::vector<Pixel> palette;

	uint8_t* imagedata = nullptr;
	size_t idata_size = 0;

	bool palette_mode = false;
};

int main(int argc, char** argv) {
	if(argc <= 1) {
		cout << "No arguments!" << endl;
		exit(1);
	}

	char* filename = argv[argc - 1];
	std::string filename_str(filename);
	std::string out;

	cout << "Input: " << filename_str << endl;

	std::size_t dotPos = filename_str.find_last_of('.');
	
    if (dotPos != std::string::npos) {
        out = filename_str.substr(0, dotPos + 1) + "duke";
    } else {
	    out = filename_str + ".duke";
    }

    cout << "Output: " << out << endl;

	Duke image(out.c_str());

	image.from_png(filename);

	cout << image << endl;

	image.save();

	return 0;
}
