use protobuf_codegen::CodeGen;

fn main() {
    CodeGen::new()
        .inputs(["proto_example/foo.proto", "proto_example/bar/bar.proto"])
        .include("proto")
        .include(protobuf_well_known_types::import_path())
        .c_include(protobuf_well_known_types::include_path())
        .option(format!("bazel_crate_mapping={}", protobuf_well_known_types::crate_mapping().display()))
        .generate_and_compile()
        .unwrap();
}
