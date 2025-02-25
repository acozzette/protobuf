use std::path::Path;
use std::path::PathBuf;

include!(concat!(env!("OUT_DIR"), "/protobuf_generated/google/protobuf/generated.rs"));

pub fn import_path() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("proto")
}

pub fn include_path() -> PathBuf {
    Path::new(env!("OUT_DIR")).join("protobuf_generated")
}

pub fn crate_mapping() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("crate_mapping.txt")
}
