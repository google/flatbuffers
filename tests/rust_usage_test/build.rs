use std::process::Command;

fn main() {
    
    let project_root = std::env::current_dir()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
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

    let out_dir = std::path::Path::new(&std::env::var("OUT_DIR").unwrap()).to_path_buf();

    Command::new(&flatc)
        .arg("--rust")
        .arg(&sample_schema)
        .output()
        .expect("Failed to generate file");

    let genfile = "monster_generated.rs";
    std::fs::rename(&genfile, out_dir.join("monster_generated.rs")).unwrap();
}
