// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// @file hello_tutorial.cc
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code.  After the code is loaded,
/// CreateModule() is not called again.
///
/// Once the code is loaded, the browser calls the CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///
/// The browser can talk to your NaCl module via the postMessage() Javascript
/// function.  When you call postMessage() on your NaCl module from the browser,
/// this becomes a call to the HandleMessage() method of your pp::Instance
/// subclass.  You can send messages back to the browser by calling the
/// PostMessage() method on your pp::Instance.  Note that these two methods
/// (postMessage() in Javascript and PostMessage() in C++) are asynchronous.
/// This means they return immediately - there is no waiting for the message
/// to be handled.  This has implications in your program design, particularly
/// when mutating property values that are exposed to both the browser and the
/// NaCl module.

#include <opencv2/opencv.hpp>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/var_dictionary.h>


bool decode_base64(const std::string& src, std::vector<unsigned char>& dst)
{
    if (src.size() & 0x00000003) {
        return false;
    }
    else {
        const std::string          table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
        std::vector<unsigned char> cdst;

        for (std::size_t i = 0; i < src.size(); i += 4) {
            if (src[i + 0] == '=') {
                return false;
            }
            else if (src[i + 1] == '=') {
                return false;
            }
            else if (src[i + 2] == '=') {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);

                if (s1 == std::string::npos || s2 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));

                break;
            }
            else if (src[i + 3] == '=') {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);
                const std::string::size_type s3 = table.find(src[i + 2]);

                if (s1 == std::string::npos || s2 == std::string::npos || s3 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));
                cdst.push_back(static_cast<unsigned char>(((s2 & 0x0F) << 4) | ((s3 & 0x3C) >> 2)));

                break;
            }
            else {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);
                const std::string::size_type s3 = table.find(src[i + 2]);
                const std::string::size_type s4 = table.find(src[i + 3]);

                if (s1 == std::string::npos || s2 == std::string::npos || s3 == std::string::npos || s4 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));
                cdst.push_back(static_cast<unsigned char>(((s2 & 0x0F) << 4) | ((s3 & 0x3C) >> 2)));
                cdst.push_back(static_cast<unsigned char>(((s3 & 0x03) << 6) | ((s4 & 0x3F) >> 0)));
            }
        }

        dst.swap(cdst);

        return true;
    }
}

// base64 エンコード
void encode_base64(const std::vector<unsigned char>& src, std::string& dst)
{
    const std::string table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    std::string       cdst;

    for (std::size_t i = 0; i < src.size(); ++i) {
        switch (i % 3) {
        case 0:
            cdst.push_back(table[(src[i] & 0xFC) >> 2]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x03) << 4]);
                cdst.push_back('=');
                cdst.push_back('=');
            }

            break;
        case 1:
            cdst.push_back(table[((src[i - 1] & 0x03) << 4) | ((src[i + 0] & 0xF0) >> 4)]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x0F) << 2]);
                cdst.push_back('=');
            }

            break;
        case 2:
            cdst.push_back(table[((src[i - 1] & 0x0F) << 2) | ((src[i + 0] & 0xC0) >> 6)]);
            cdst.push_back(table[src[i] & 0x3F]);

            break;
        }
    }

    dst.swap(cdst);
}

// May return null image (.data == nullptr)
cv::Mat convertDataURLToImage(const std::string& url) {
    const std::string input_b64 = url.substr(url.find(",") + 1);
    std::vector<uint8_t> input_data;
    decode_base64(input_b64, input_data);

    return cv::imdecode(cv::Mat(input_data), CV_LOAD_IMAGE_COLOR);
}

std::string convertImageToDataURL(const cv::Mat& image) {
    std::vector<uint8_t> buffer;
    cv::imencode(".jpg", image, buffer);

    std::string buffer_b64;
    encode_base64(buffer, buffer_b64);

    return "data:image/jpeg;base64," + buffer_b64;
}

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
///     src="hello_tutorial.nmf"
///     type="application/x-pnacl"
/// To communicate with the browser, you must override HandleMessage() to
/// receive messages from the browser, and use PostMessage() to send messages
/// back to the browser.  Note that this interface is asynchronous.
class HelloTutorialInstance : public pp::Instance {
public:
    /// The constructor creates the plugin-side instance.
    /// @param[in] instance the handle to the browser-side plugin instance.
    explicit HelloTutorialInstance(PP_Instance instance) : pp::Instance(instance) {
    }

    virtual ~HelloTutorialInstance() {
    }

    /// Handler for messages coming in from the browser via postMessage().  The
    /// @a var_message can contain be any pp:Var type; for example int, string
    /// Array or Dictinary. Please see the pp:Var documentation for more details.
    /// @param[in] var_message The message posted by the browser.
    virtual void HandleMessage(const pp::Var& var_message);
private:
    pp::VarDictionary handleNewImage(cv::Mat image);
    pp::VarDictionary handleCalibration();

    void log(std::string message);
private:
    std::vector<std::vector<cv::Vec2f>> sets_points_image;
    std::vector<std::vector<cv::Vec3f>> sets_points_world;
};


/// Handler for messages coming in from the browser via postMessage().  The
/// @a var_message can contain be any pp:Var type; for example int, string
/// Array or Dictinary. Please see the pp:Var documentation for more details.
/// @param[in] var_message The message posted by the browser.
void HelloTutorialInstance::HandleMessage(const pp::Var& var_message) {
    if(!var_message.is_dictionary()) {
        return;
    }
    const pp::VarDictionary& dict_message = reinterpret_cast<const pp::VarDictionary&>(var_message);
    const std::string message_type = dict_message.Get("type").AsString();

    if(message_type == "process_image") {
        cv::Mat input = convertDataURLToImage(dict_message.Get("data").AsString());
        if(!input.data) {
            return;
        }

        PostMessage(handleNewImage(input));
    } else if(message_type == "calibrate") {
        PostMessage(handleCalibration());
    } else {
        log("Unknown message type: " + message_type);
    }
}

pp::VarDictionary HelloTutorialInstance::handleNewImage(cv::Mat input) {
    log("Image Received");
    // Process
    std::vector<cv::Vec2f> points_v;
    const bool recog_success = cv::findChessboardCorners(input, cv::Size(8, 6), points_v);

    log("Found corners " + std::to_string(points_v.size()));

    cv::drawChessboardCorners(input, cv::Size(8, 6), points_v, recog_success);
    log("drawn corners");

    if(recog_success) {
        sets_points_image.push_back(points_v);

        std::vector<cv::Vec3f> points_world;
        const float size = 0.03;  // meter
        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 8; j++) {
                points_world.push_back(cv::Vec3f(i * size, j * size, 0));
            }
        }
        sets_points_world.push_back(points_world);
    }

    // Write image as data URI.
    pp::VarDictionary reply;
    reply.Set("type", "image_result");
    reply.Set("success", recog_success);
    reply.Set("image_url", convertImageToDataURL(input));
    return reply;
}

pp::VarDictionary HelloTutorialInstance::handleCalibration() {
    const int camera_width = 320;
    const int camera_height = 240;

    assert(sets_points_image.size() == sets_points_world.size());
    assert(sets_points_image[0].size() == sets_points_world[0].size());

    log("Start calibrating");
    cv::Mat_<float> intrinsic;
    std::vector<cv::Mat> rotations;
    std::vector<cv::Mat> translations;
    cv::Mat_<float> coeff;

    const double error = cv::calibrateCamera(
        sets_points_world, sets_points_image,
        cv::Size(camera_width, camera_height),
        intrinsic, coeff, rotations, translations);

    pp::VarDictionary reply;
    reply.Set("type", "calibration");

    reply.Set("error", error);

    pp::VarDictionary result_intrinsic;
    result_intrinsic.Set("width", camera_width);
    result_intrinsic.Set("height", camera_height);
    result_intrinsic.Set("fx", intrinsic(0, 0));
    result_intrinsic.Set("fy", intrinsic(1, 1));
    result_intrinsic.Set("cx", intrinsic(0, 2));
    result_intrinsic.Set("cy", intrinsic(1, 2));

    pp::VarDictionary result_distortion;
    result_distortion.Set("type", "radial+tangential");
    result_distortion.Set("k1", coeff(0));
    result_distortion.Set("k2", coeff(1));
    result_distortion.Set("p1", coeff(2));
    result_distortion.Set("p2", coeff(3));

    result_intrinsic.Set("distortion", result_distortion);

    reply.Set("intrinsic", result_intrinsic);
    return reply;
}

void HelloTutorialInstance::log(std::string message) {
    pp::VarDictionary reply;
    reply.Set("type", "debug");
    reply.Set("message", message);
    PostMessage(reply);
}

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-pnacl".
class HelloTutorialModule : public pp::Module {
public:
    HelloTutorialModule() : pp::Module() {}
    virtual ~HelloTutorialModule() {}

    /// Create and return a HelloTutorialInstance object.
    /// @param[in] instance The browser-side instance.
    /// @return the plugin-side instance.
    virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new HelloTutorialInstance(instance);
    }
};

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
Module* CreateModule() {
    return new HelloTutorialModule();
}
}  // namespace pp
