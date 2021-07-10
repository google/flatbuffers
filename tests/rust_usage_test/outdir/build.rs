fn main() {
    use std::process::Command;
    let project_root = std::env::current_dir()
        .unwrap()
        .parent()  // flatbuffers/tests/rust_usage test
        .unwrap()
        .parent()  // flatbuffers/tests
        .unwrap()
        .parent()  // flatbuffers/
        .unwrap()
        .to_path_buf();
    let sample_schema = {
        let mut s = project_root.to_path_buf();
        s.push("samples");
        s.push("monster.fbs");
        s
    };

    let flatc = {
        let mut f = project_root.to_path_buf();
        f.push("flatc");
        f
    };

    // DO NOT SUBMIT: CASPER: Can you put generated code in outdir?
    Command::new(&flatc)
        .arg("-o")
        .arg("src/generated/")
        .arg("--rust")
        .arg(&sample_schema)
        .output()
        .expect("Failed to generate file");
}
