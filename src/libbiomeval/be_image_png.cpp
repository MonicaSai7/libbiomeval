/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <png.h>

#include <be_image_png.h>
#include <be_memory.h>
#include <be_memory_autoarray.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

/** Wrapper for reading PNG-encoded data from an AutoArray with libpng. */
struct png_buffer
{
	/** PNG-encoded buffer */
	const uint8_t *data;
	/** Size of data */
	const uint64_t size;
	/** Number of bytes currently read by libpng */
	png_size_t offset;

};
using png_buffer = struct png_buffer;

/**
 * @brief
 * libpng callback to read data from an AutoArray.
 *
 * @param png_ptr
 * Pointer to a PNG struct for the image.
 * @param buffer
 * Pointer to a png_buffer struct.
 * @param length
 * Amount of data to copy from buffer->data into buffer, starting from
 * buffer->offset.
 *
 * @throw Error::StrategyError
 * Input buffer given to libpng is nullptr or libpng wants to read more data
 * from input buffer than available.
 */
static void
png_read_mem_src(
    png_structp png_ptr,
    png_bytep buffer,
    png_size_t length);

/**
 * @brief
 * Call statusCallback from libpng errors.
 *
 * @param png_ptr
 * Pointer to a PNG struct for the image.
 * @param msg
 * C-style string containing an error message, generated by libpng.
 *
 * @throw Error::StrategyError
 * Always thrown with msg, even if statusCallback won't
 */
static void
png_error_callback(
    png_structp png_ptr,
    png_const_charp msg);

/**
 * @brief
 * Call statusCallback from libpng warnings.
 *
 * @param png_ptr
 * Pointer to a PNG struct for the image.
 * @param msg
 * C-style string containing an error message, generated by libpng.
 */
static void
png_warning_callback(
    png_structp png_ptr,
    png_const_charp msg);

BiometricEvaluation::Image::PNG::PNG(
    const uint8_t *data,
    const uint64_t size,
    const std::string &identifier,
    const statusCallback_t &statusCallback) :
    Image::Image(
    data,
    size,
    CompressionAlgorithm::PNG,
    identifier,
    statusCallback)
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    (void *)this, png_error_callback, png_warning_callback);
	if (png_ptr == nullptr)
		throw Error::StrategyError("Could not initialize reading");

	/* Read encoded PNG data from an AutoArray using our extension */
	png_buffer png_buf = { this->getDataPointer(), this->getDataSize(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);

	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == nullptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		throw Error::StrategyError("Could not initialize container for "
		    "information");
	}
	png_read_info(png_ptr, png_info_ptr);

	setColorDepth(png_get_bit_depth(png_ptr, png_info_ptr) *
	    png_get_channels(png_ptr, png_info_ptr));
	this->setBitDepth(png_get_bit_depth(png_ptr, png_info_ptr));
	setDimensions(Size(png_get_image_width(png_ptr, png_info_ptr),
	    png_get_image_height(png_ptr, png_info_ptr)));

	png_uint_32 xres, yres;
	int32_t type;
	if (png_get_pHYs(png_ptr, png_info_ptr, &xres, &yres, &type) ==
	    PNG_INFO_pHYs) {
	    	switch (type) {
		case PNG_RESOLUTION_METER:
			setResolution(Resolution(xres / 100.0, yres / 100.0,
			    Resolution::Units::PPCM));
			break;
		case PNG_RESOLUTION_UNKNOWN:
			/* Resolution based on aspect ratio */
		case PNG_RESOLUTION_LAST:
			/* FALLTHROUGH */
		default:
			/* FALLTHROUGH */
			/*
			 * For our purposes, there really is no good way to
			 * unambiguously set a resolution.
			 */
			setResolution(Resolution(0, 0,
			    Resolution::Units::PPCM));
			break;
		}
	} else {
		/*
		 * Assume resolution is 72 dpi on both axis if resolution
		 * is not set, which is often the case in order to reduce
		 * file size.
		 */
		setResolution(Resolution(72, 72, Resolution::Units::PPI));
	}
	png_byte color_type{png_get_color_type(png_ptr, png_info_ptr)};
	this->setHasAlphaChannel((color_type & PNG_COLOR_MASK_ALPHA) ==
	    PNG_COLOR_MASK_ALPHA);

	png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
}

BiometricEvaluation::Image::PNG::PNG(
    const BiometricEvaluation::Memory::uint8Array &data,
    const std::string &identifier,
    const statusCallback_t &statusCallback) :
    BiometricEvaluation::Image::PNG::PNG(
    data,
    data.size(),
    identifier,
    statusCallback)
{

}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::PNG::getRawData()
    const
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    (void *)this, png_error_callback, png_warning_callback);
	if (png_ptr == nullptr)
		throw Error::StrategyError("Could not initialize reading");

	/* Read encoded PNG data from a buffer using our extension */
	png_buffer png_buf = { this->getDataPointer(), this->getDataSize(), 0 };
	png_set_read_fn(png_ptr, &png_buf, png_read_mem_src);

	/* Read the header information */
	png_infop png_info_ptr = png_create_info_struct(png_ptr);
	if (png_info_ptr == nullptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		throw Error::StrategyError("Could not initialize container for "
		    "information");
	}
	png_read_info(png_ptr, png_info_ptr);

	/* PNG default storage is big-endian */
	if ((png_get_bit_depth(png_ptr, png_info_ptr) > 8) &&
	    BiometricEvaluation::Memory::isLittleEndian())
		png_set_swap(png_ptr);

	/* Determine size of decompressed data */
	const png_uint_32 rowbytes = png_get_rowbytes(png_ptr, png_info_ptr);
	const uint32_t height = this->getDimensions().ySize;
	Memory::AutoArray<png_bytep> row_pointers(height);
	Memory::uint8Array rawData(rowbytes * height);

	/* Tell libpng to store decompressed PNG data directly into AutoArray */
	for (uint32_t row = 0; row < height; row++)
		row_pointers[row] = rawData + (row * rowbytes);
	png_read_image(png_ptr, row_pointers);

	/* Check for palette color (1, 2, 4, 8-bit depth only) */
	if (this->getBitDepth() > 8) {
		png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
		return (rawData);
	}
	png_byte color_type{png_get_color_type(png_ptr, png_info_ptr)};
	if ((color_type & PNG_COLOR_MASK_PALETTE) != PNG_COLOR_MASK_PALETTE) {
		png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
		return (rawData);
	}

	/* Read and parse the palette data */
	png_colorp palette{};
	int paletteSize{};
	if (png_get_PLTE(png_ptr, png_info_ptr, &palette, &paletteSize) !=
	    PNG_INFO_PLTE) {
	    	png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
		throw BE::Error::StrategyError("Expected palette data, but no "
		    "PLTE chunk found");
	}

	const bool paletteIsGrayscale = std::all_of(&palette[0],
	    &palette[paletteSize], [](const png_color &c) -> bool {
		return ((c.red == c.green) && (c.green == c.blue));
	});

	if (paletteIsGrayscale) {
		std::for_each(rawData.begin(), rawData.end(), [&](uint8_t &c) {
			c = palette[c].red;
		});
	} else {
		BE::Memory::uint8Array expandedRawData(rawData.size() * 3);
		BE::Memory::MutableIndexedBuffer buf{expandedRawData};

		std::for_each(rawData.begin(), rawData.end(),
		    [&](const uint8_t &c) {
			const auto paletteColor = palette[c];
			buf.pushU8Val(paletteColor.red);
			buf.pushU8Val(paletteColor.green);
			buf.pushU8Val(paletteColor.blue);
		});

		rawData = expandedRawData;
	}

	png_free_data(png_ptr, png_info_ptr, PNG_FREE_PLTE, 0);
	png_destroy_read_struct(&png_ptr, &png_info_ptr, nullptr);
	return (rawData);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Image::PNG::getRawGrayscaleData(
    uint8_t depth)
    const
{
	return (Image::getRawGrayscaleData(depth));
}

bool
BiometricEvaluation::Image::PNG::isPNG(
    const uint8_t *data,
    uint64_t size)
{
	static const png_size_t PNG_SIG_LENGTH = 8;
	if (size <= PNG_SIG_LENGTH)
		return (false);

	png_byte header[PNG_SIG_LENGTH];
	std::memcpy(header, data, PNG_SIG_LENGTH);
	return (png_sig_cmp(header, 0, PNG_SIG_LENGTH) == 0);
}

void
png_read_mem_src(
    png_structp png_ptr,
    png_bytep buffer,
    png_size_t length)
{
	// XXX: It appears that libpng calls this function in 4-byte
	// increments, which can't be good for performance.  See if there is
	// a way to increase the chunk size.  Ideally, one would set the chunk
	// size to the size of the buffer since it is known ahead of time.

	if (png_get_io_ptr(png_ptr) == nullptr)
		throw BE::Error::StrategyError("Lost current offset while "
		    "reading from memory");

	png_buffer *input = (png_buffer *)png_get_io_ptr(png_ptr);
	if (length > (input->size - input->offset))
		throw BE::Error::StrategyError("Attempted to read more data "
		    "than available");

	memcpy(buffer, input->data + input->offset, length);
	input->offset += length;
}

void
png_error_callback(
    png_structp png_ptr,
    png_const_charp msg)
{
	const void *userData = png_get_error_ptr(png_ptr);
	if (userData != nullptr) {
		const BE::Image::PNG *png = static_cast<const BE::Image::PNG*>(
		    userData);
		png->getStatusCallback()({BE::Framework::Status::Type::Error,
		    msg, png->getIdentifier()});
	}

	/*
	 * libpng cannot continue after errors, so if statusCallback won't
	 * throw, we will.
	 */
	throw BE::Error::StrategyError(msg);
}

void
png_warning_callback(
    png_structp png_ptr,
    png_const_charp msg)
{
	const void *userData = png_get_error_ptr(png_ptr);
	if (userData == nullptr)
		return;

	const BE::Image::PNG *png = static_cast<const BE::Image::PNG*>(
	    userData);
	png->getStatusCallback()({BE::Framework::Status::Type::Error, msg,
	    png->getIdentifier()});
}
