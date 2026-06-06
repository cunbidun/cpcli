{
  pkgs,
  root,
}:
let
  rustToolchain = pkgs.rust-bin.stable.latest.default.override {
    extensions = ["rust-src" "rust-analyzer"];
  };

  rustPlatform = pkgs.makeRustPlatform {
    cargo = rustToolchain;
    rustc = rustToolchain;
  };

  src = pkgs.lib.cleanSourceWith {
    src = root + /default/task_editor/tauri_task_editor;
    filter = path: type:
      let
        base = baseNameOf path;
      in
        !(base == "node_modules"
          || base == "dist"
          || base == "target"
          || base == "result");
  };

  libraries = pkgs.lib.optionals pkgs.stdenv.isLinux (with pkgs; [
    webkitgtk_4_1
    gtk3
    cairo
    gdk-pixbuf
    glib
    dbus
    openssl
    librsvg
    gst_all_1.gstreamer
    gst_all_1.gst-plugins-base
  ]);
in
  rustPlatform.buildRustPackage {
    pname = "task-editor";
    version = "1.0.0";
    inherit src;

    buildAndTestSubdir = "src-tauri";

    cargoLock = {
      lockFile = root + /default/task_editor/tauri_task_editor/src-tauri/Cargo.lock;
    };

    # Older nixpkgs revisions ship cargo-auditable versions that cannot parse
    # Rust 2024 metadata emitted by the rust-overlay toolchain.
    auditable = false;

    npmDeps = pkgs.fetchNpmDeps {
      inherit src;
      hash = "sha256-Mbk860fHkcjt/++1N/L+idzYbd/iStzxshIpTsNCPDU=";
    };

    nativeBuildInputs = with pkgs;
      [
        pkg-config
        nodejs
        npmHooks.npmConfigHook
      ]
      ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
        wrapGAppsHook3
      ];

    buildInputs = libraries;

    preFixup = pkgs.lib.optionalString pkgs.stdenv.isLinux ''
      gappsWrapperArgs+=(
        --set WEBKIT_DISABLE_DMABUF_RENDERER 1
        --set GIO_USE_VFS local
        --set GTK_IM_MODULE gtk-im-context-simple
      )
    '';

    postPatch = ''
      cp src-tauri/Cargo.lock Cargo.lock
    '';

    preBuild = ''
      (cd "$NIX_BUILD_TOP/source" && npm run build)
    '';
  }
