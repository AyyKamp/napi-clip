#include "../cpp_modules/clip/clip.h"
#include <iostream>

#include <napi.h>
using namespace Napi;

Boolean clear(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	return Boolean::New(env, clip::clear());
}

String get_text(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	std::string value;
	clip::get_text(value);
  return String::New(env, value);
}

Boolean set_text(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	if (args.Length() < 1 || !args[0].IsString()) {
    Error::New(env, "Invalid argument provided. Must be of type string.").ThrowAsJavaScriptException();
    return Boolean::New(env, false);
  }
	
	std::string value = args[0].ToString();
	return Boolean::New(env, clip::set_text(value));
}

Boolean has_image(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	bool has_image_value = clip::has(clip::image_format());
	
	return Boolean::New(env, has_image_value);
}

Boolean has_text(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	bool has_text_value = clip::has(clip::text_format());
	
	return Boolean::New(env, has_text_value);
}
// https://github.com/dacap/clip/issues/28
/* Boolean is_empty(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	bool has_empty_format = clip::has(clip::empty_format());
	
	return Boolean::New(env, has_empty_format);
} */

Object get_image(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	if (!clip::has(clip::image_format())) {
		Error::New(env, "No image in clipboard.").ThrowAsJavaScriptException();
		return Object::New(env);
	}

	clip::image img;
	if (!clip::get_image(img)) {
		Error::New(env, "Error getting image from clipboard.").ThrowAsJavaScriptException();
		return Object::New(env);
	}

	clip::image_spec spec = img.spec();

	Object img_obj = Object::New(env);
	Object spec_obj = Object::New(env);

	// this may be imporved, not very DRY
	spec_obj.Set(String::New(env, "width"), Number::New(env, spec.width));
	spec_obj.Set(String::New(env, "height"), Number::New(env, spec.height));
	spec_obj.Set(String::New(env, "bitsPerPixel"), Number::New(env, spec.bits_per_pixel));
	spec_obj.Set(String::New(env, "bytesPerRow"), Number::New(env, spec.bytes_per_row));
	spec_obj.Set(String::New(env, "redMask"), Number::New(env, spec.red_mask));
	spec_obj.Set(String::New(env, "greenMask"), Number::New(env, spec.green_mask));
	spec_obj.Set(String::New(env, "blueMask"), Number::New(env, spec.blue_mask));
	spec_obj.Set(String::New(env, "alphaMask"), Number::New(env, spec.alpha_mask));
	spec_obj.Set(String::New(env, "redShift"), Number::New(env, spec.red_shift));
	spec_obj.Set(String::New(env, "greenShift"), Number::New(env, spec.green_shift));
	spec_obj.Set(String::New(env, "blueShift"), Number::New(env, spec.blue_shift));
	spec_obj.Set(String::New(env, "alphaShift"), Number::New(env, spec.alpha_shift));

	img_obj.Set(String::New(env, "spec"), spec_obj);

	const size_t length = spec.width * spec.height; 
	
	char *data = img.data();
	Buffer<char> img_buffer = Buffer<char>::Copy(env, data, length * (spec.bits_per_pixel / 8));

	img_obj.Set(String::New(env, "data"), img_buffer);

	return img_obj;
}

Boolean set_image(const CallbackInfo& args) {
	Napi::Env env = args.Env();

	if (args.Length() < 1 || !args[0].IsObject()) {
		Error::New(env, "Invalid argument provided. Must be object").ThrowAsJavaScriptException();
		return Boolean::New(env, false);
	}

	Object obj = args[0].ToObject();

	Napi::Buffer<char> img_buffer = obj.Get("data").As<Buffer<char>>();
	char* img_data = img_buffer.Data();

	clip::image_spec spec;
	Object spec_obj = obj.Get("spec").ToObject();

	// very bad code never hire this guy
	spec.width = spec_obj.Has("width") ?
		 spec_obj.Get("width").ToNumber().Uint32Value(): 0;
	spec.height = spec_obj.Has("height") ?
		 spec_obj.Get("height").ToNumber().Uint32Value(): 0;
	spec.bits_per_pixel = spec_obj.Has("bitsPerPixel") ?
		 spec_obj.Get("bitsPerPixel").ToNumber().Uint32Value(): 32;
	spec.bytes_per_row = spec_obj.Has("bytesPerRow") ?
		 spec_obj.Get("bytesPerRow").ToNumber().Uint32Value(): spec.width * 4;
	spec.red_mask = spec_obj.Has("redMask") ?
		 spec_obj.Get("redMask").ToNumber().Uint32Value()		: 0xff;
	spec.green_mask = spec_obj.Has("greenMask") ?
		 spec_obj.Get("greenMask").ToNumber().Uint32Value()	: 0xff00;
	spec.blue_mask = spec_obj.Has("blueMask") ?
		 spec_obj.Get("blueMask").ToNumber().Uint32Value() 	: 0xff0000;
	spec.alpha_mask = spec_obj.Has("alphaMask") ?
		 spec_obj.Get("alphaMask").ToNumber().Uint32Value() : 0xff000000;
	spec.red_shift = spec_obj.Has("redShift") ?
		 spec_obj.Get("redShift").ToNumber().Uint32Value() 	: 0;
	spec.green_shift = spec_obj.Has("greenShift") ?
		 spec_obj.Get("greenShift").ToNumber().Uint32Value(): 8;
	spec.blue_shift = spec_obj.Has("blueShift") ?
		 spec_obj.Get("blueShift").ToNumber().Uint32Value()	: 16;
	spec.alpha_shift = spec_obj.Has("alphaShift") ?
		 spec_obj.Get("alphaShift").ToNumber().Uint32Value(): 24;
	
	clip::image img(img_data, spec);
	return Boolean::New(env, clip::set_image(img));
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
	exports.Set("getText", Napi::Function::New(env, get_text));
	exports.Set("setText", Napi::Function::New(env, set_text));
	exports.Set("hasText", Napi::Function::New(env, has_text));
	exports.Set("hasImage", Napi::Function::New(env, has_image));
	// exports.Set("isEmpty", Napi::Function::New(env, is_empty));
	exports.Set("getImage", Napi::Function::New(env, get_image));
	exports.Set("setImage", Napi::Function::New(env, set_image));
	exports.Set("clear", Napi::Function::New(env, clear));
	return exports;
}
	
NODE_API_MODULE(napi_clip, Initialize)
