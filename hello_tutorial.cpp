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
  explicit HelloTutorialInstance(PP_Instance instance) : pp::Instance(instance)
  {}
  virtual ~HelloTutorialInstance() {}

  /// Handler for messages coming in from the browser via postMessage().  The
  /// @a var_message can contain be any pp:Var type; for example int, string
  /// Array or Dictinary. Please see the pp:Var documentation for more details.
  /// @param[in] var_message The message posted by the browser.
  virtual void HandleMessage(const pp::Var& var_message) {
    if(!var_message.is_dictionary()) {
      return;
    }
    const pp::VarDictionary& dict_message = reinterpret_cast<const pp::VarDictionary&>(var_message);

    // Load image from data URI.
    const std::string input_data_url = dict_message.Get("data").AsString();
    const std::string input_b64 = input_data_url.substr(input_data_url.find(",") + 1);
    std::vector<uint8_t> input_data;
    decode_base64(input_b64, input_data);

    const cv::Mat input = cv::imdecode(cv::Mat(input_data), CV_LOAD_IMAGE_COLOR);
    if(!input.data) {
      return;
    }

    // Process
    cv::Mat output;
    cv::bilateralFilter(input, output, 0, 7, 7);

    // Write image as data URI.
    std::vector<uint8_t> buffer;
    cv::imencode(".jpg", output, buffer);

    std::string buffer_b64;
    encode_base64(buffer, buffer_b64);

    const std::string data_url = "data:image/jpeg;base64," + buffer_b64;
    PostMessage(pp::Var(data_url));
  }
};

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
