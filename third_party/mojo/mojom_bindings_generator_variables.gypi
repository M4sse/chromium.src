# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'mojom_bindings_generator':
        '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/mojom_bindings_generator.py',
    'mojom_bindings_generator_sources': [
      '<(mojom_bindings_generator)',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/enum_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_macros.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_proxy_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_request_validator_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_response_validator_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/interface_stub_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/module.cc.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/module.h.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/module-internal.h.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/struct_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/struct_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/struct_macros.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/struct_serialization_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/wrapper_class_declaration.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/cpp_templates/wrapper_class_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/constant_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/constants.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/enum_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/enum.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/header.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/interface_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/interface.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/interface_internal.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/struct_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/java_templates/struct.java.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/js_templates/enum_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/js_templates/interface_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/js_templates/module_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/js_templates/module.amd.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/js_templates/struct_definition.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/python_templates/module_macros.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/python_templates/module.py.tmpl',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/mojom_cpp_generator.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/mojom_java_generator.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/mojom_js_generator.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/generators/mojom_python_generator.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/__init__.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/error.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/__init__.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/data.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/generator.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/module.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/pack.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/generate/template_expander.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/parse/__init__.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/parse/ast.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/parse/lexer.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/parse/parser.py',
      '<(DEPTH)/third_party/mojo/src/mojo/public/tools/bindings/pylib/mojom/parse/translate.py',
    ]
  }
}
