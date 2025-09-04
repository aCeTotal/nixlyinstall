{
  description = "Wayland Qt6 C++ NixlyInstall App";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.qt6.qtbase
            pkgs.qt6.qtwayland
            pkgs.qt6.qttools
            pkgs.qt6.qtsvg
            pkgs.wayland
            pkgs.libxkbcommon
            pkgs.meson
            pkgs.ninja
            pkgs.pkg-config
            pkgs.cmake
            pkgs.gcc
            pkgs.gdb
            pkgs.wayland-protocols
            pkgs.wayland-scanner
            pkgs.wayland-utils
            pkgs.weston
            pkgs.libglvnd
          ];
          shellHook = ''
            export QT_QPA_PLATFORM=wayland
            export QT_QPA_PLATFORMTHEME=wayland
            export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
            
            export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins:${pkgs.qt6.qtwayland}/lib/qt-6/plugins
            export QT_QPA_PLATFORM_PLUGIN_PATH=${pkgs.qt6.qtwayland}/lib/qt-6/plugins/platforms
            
            export QT_WAYLAND_SHELL_INTEGRATION=xdg-shell
            
            export QT_DEBUG_PLUGINS=1
            export QT_LOGGING_RULES="qt.qpa.*=true"
            
            echo "Qt plugin path: $QT_PLUGIN_PATH"
            echo "Qt platform plugin path: $QT_QPA_PLATFORM_PLUGIN_PATH"
          '';
        };
      }
    );
}
