use wasm_bindgen::prelude::*;

#[wasm_bindgen]
pub fn resize(
    input_image: Vec<u32>,
    input_width: usize,
    input_height: usize,
    factor: usize,
) -> Vec<u32> {
    let mut output_image = vec![0u32; input_width * input_height * factor * factor];

    match factor {
        2 => hqx::hq2x(&input_image, &mut output_image, input_width, input_height),
        3 => hqx::hq3x(&input_image, &mut output_image, input_width, input_height),
        4 => hqx::hq4x(&input_image, &mut output_image, input_width, input_height),
        _ => unreachable!(),
    };

    output_image
}
