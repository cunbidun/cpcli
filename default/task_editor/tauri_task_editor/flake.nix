{
  description = "Tauri Task Editor for cpcli";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    rust-overlay,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      overlays = [(import rust-overlay)];
      pkgs = import nixpkgs {
        inherit system overlays;
      };

      rustToolchain = pkgs.rust-bin.stable.latest.default.override {
        extensions = ["rust-src" "rust-analyzer"];
      };

      # Common dependencies for Tauri (Linux only - macOS uses system frameworks)
      libraries = pkgs.lib.optionals pkgs.stdenv.isLinux (with pkgs; [
        webkitgtk_4_1
        gtk3
        cairo
        gdk-pixbuf
        glib
        dbus
        openssl
        librsvg
      ]);

      buildInputs = with pkgs;
        [
          rustToolchain
          cargo
          pkg-config
          nodejs
        ]
        ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
          # Linux/GTK Tauri dependencies
          webkitgtk_4_1
          gtk3
          cairo
          gdk-pixbuf
          glib
          dbus
          openssl
          librsvg
          libsoup_3
        ]
        ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
          # macOS uses system frameworks, just need libiconv
          libiconv
        ];
    in {
      devShells.default = pkgs.mkShell {
        inherit buildInputs;

        shellHook = ''
          ${pkgs.lib.optionalString pkgs.stdenv.isLinux ''
            export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath libraries}:$LD_LIBRARY_PATH
            export XDG_DATA_DIRS=${pkgs.gsettings-desktop-schemas}/share/gsettings-schemas/${pkgs.gsettings-desktop-schemas.name}:${pkgs.gtk3}/share/gsettings-schemas/${pkgs.gtk3.name}:$XDG_DATA_DIRS
            export GIO_MODULE_DIR="${pkgs.glib-networking}/lib/gio/modules/"
          ''}
        '';
      };

      packages.default = pkgs.rustPlatform.buildRustPackage {
        pname = "task-editor";
        version = "1.0.0";
        src = ./src-tauri;

        cargoLock = {
          lockFile = ./src-tauri/Cargo.lock;
        };

        nativeBuildInputs = with pkgs;
          [
            pkg-config
            nodejs_20
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
            wrapGAppsHook
          ];

        buildInputs = libraries;

        preBuild = ''
          cd ..
          npm ci
          npm run build
          cd src-tauri
        '';
      };
    });
}
