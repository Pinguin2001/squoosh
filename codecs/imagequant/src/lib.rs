use imagequant::RGBA;
use wasm_bindgen::prelude::*;
use wasm_bindgen::Clamped;

fn to_pixels(data: &[u8]) -> Vec<RGBA> {
    data.chunks_exact(4)
        .map(|p| RGBA::new(p[0], p[1], p[2], p[3]))
        .collect()
}

fn to_bytes(pixels: &[RGBA]) -> Vec<u8> {
    pixels.iter().flat_map(|c| [c.r, c.g, c.b, c.a]).collect()
}

/// Quantizes `pixels` to the attributes' colours and returns them as RGBA.
fn quantize_pixels(
    attr: &imagequant::Attributes,
    image: &mut imagequant::Image,
    dithering: f32,
) -> Result<Vec<RGBA>, imagequant::Error> {
    let mut res = attr.quantize(image)?;
    res.set_dithering_level(dithering)?;
    let (palette, indexes) = res.remapped(image)?;
    Ok(indexes.iter().map(|&i| palette[i as usize]).collect())
}

#[wasm_bindgen]
pub fn quantize(
    data: &[u8],
    width: usize,
    height: usize,
    num_colors: u32,
    dithering: f32,
) -> Result<Clamped<Vec<u8>>, JsError> {
    let mut attr = imagequant::new();
    attr.set_max_colors(num_colors)?;
    let mut image = attr.new_image(to_pixels(data), width, height, 0.0)?;
    let result = quantize_pixels(&attr, &mut image, dithering)?;
    Ok(Clamped(to_bytes(&result)))
}

const fn zx(r: u8, g: u8, b: u8) -> RGBA {
    RGBA { r, g, b, a: 255 }
}

const ZX_COLORS: [RGBA; 15] = [
    zx(0, 0, 0),       // regular black
    zx(0, 0, 215),     // regular blue
    zx(215, 0, 0),     // regular red
    zx(215, 0, 215),   // regular magenta
    zx(0, 215, 0),     // regular green
    zx(0, 215, 215),   // regular cyan
    zx(215, 215, 0),   // regular yellow
    zx(215, 215, 215), // regular white
    zx(0, 0, 255),     // bright blue
    zx(255, 0, 0),     // bright red
    zx(255, 0, 255),   // bright magenta
    zx(0, 255, 0),     // bright green
    zx(0, 255, 255),   // bright cyan
    zx(255, 255, 0),   // bright yellow
    zx(255, 255, 255), // bright white
];

/// The ZX has one bit per pixel, but can assign two colours to an 8x8 block.
/// The two colours must both be 'regular' or 'bright'. Black exists as both
/// regular and bright.
#[wasm_bindgen]
pub fn zx_quantize(
    data: &[u8],
    width: usize,
    height: usize,
    dithering: f32,
) -> Result<Clamped<Vec<u8>>, JsError> {
    let pixels = to_pixels(data);
    let mut result = vec![RGBA::new(0, 0, 0, 0); width * height];

    for block_y in (0..height).step_by(8) {
        for block_x in (0..width).step_by(8) {
            // Blocks hanging off the right/bottom of the image are smaller.
            let block_width = 8.min(width - block_x);
            let block_height = 8.min(height - block_y);
            let mut block = Vec::with_capacity(block_width * block_height);
            let mut popularity = [0i32; 15];

            for y in block_y..block_y + block_height {
                for x in block_x..block_x + block_width {
                    let pixel = pixels[y * width + x];
                    block.push(pixel);

                    // Which ZX colour is this pixel closest to? (Euclidean
                    // distance; the first one wins a tie.)
                    let distance = |c: &RGBA| {
                        let d = |a: u8, b: u8| (i32::from(a) - i32::from(b)).pow(2);
                        d(c.r, pixel.r) + d(c.g, pixel.g) + d(c.b, pixel.b)
                    };
                    let mut winner = 0;
                    for (index, color) in ZX_COLORS.iter().enumerate() {
                        if distance(color) < distance(&ZX_COLORS[winner]) {
                            winner = index;
                        }
                    }
                    popularity[winner] += 1;
                }
            }

            // The three most popular colours for the block.
            let (mut first, mut second, mut third) = (0, 0, 0);
            let (mut first_count, mut second_count, mut third_count) = (-1, -1, -1);
            for (index, &count) in popularity.iter().enumerate() {
                if count > first_count {
                    (third, third_count) = (second, second_count);
                    (second, second_count) = (first, first_count);
                    (first, first_count) = (index, count);
                } else if count > second_count {
                    (third, third_count) = (second, second_count);
                    (second, second_count) = (index, count);
                } else if count > third_count {
                    (third, third_count) = (index, count);
                }
            }

            // ZX images can't mix bright and regular colours, except black
            // which appears in both. Resolve any conflict:
            loop {
                // If either colour is black, there's no conflict to resolve.
                if first != 0 && second != 0 {
                    if first >= 8 && second < 8 {
                        // Make the second colour bright
                        second += 7;
                    } else if first < 8 && second >= 8 {
                        // Make the second colour regular
                        second -= 7;
                    }
                }
                // If we now have the same colour twice (bright and regular
                // versions of it), retry with the third most popular colour.
                if first == second {
                    second = third;
                } else {
                    break;
                }
            }

            let mut attr = imagequant::new();
            attr.set_max_colors(2)?;
            let mut image = attr.new_image(block, block_width, block_height, 0.0)?;
            image.add_fixed_color(ZX_COLORS[first])?;
            image.add_fixed_color(ZX_COLORS[second])?;
            let quantized = quantize_pixels(&attr, &mut image, dithering)?;

            for y in 0..block_height {
                for x in 0..block_width {
                    result[(block_y + y) * width + block_x + x] = quantized[y * block_width + x];
                }
            }
        }
    }

    Ok(Clamped(to_bytes(&result)))
}
