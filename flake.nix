{
  description = "Autonix - Autonomous Driving Simulation Environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "adas-qt6-dev";

          buildInputs = with pkgs; [
            # Build tools
            cmake
            ninja
            pkg-config
            gcc13

            # Qt6
            qt6.qtbase
            qt6.qtwayland
            qt6.wrapQtAppsHook

            # Testing
            gtest

            # XML parsing
            pugixml

            # Development utilities
            gdb
            valgrind
            lcov
            gcovr
          ];

          shellHook = ''
            echo "ADAS Qt6 Development Environment"
            echo "================================="
            echo "Qt6 Version: $(qmake6 --version | head -n1)"
            echo ""
            echo "Build commands:"
            echo "  cmake -B build -G Ninja"
            echo "  cmake --build build"
            echo "  ctest --test-dir build"
            echo ""
            export QT_QPA_PLATFORM=xcb
            export DISPLAY=:0
          '';

          # Ensure Qt can find plugins
          QT_PLUGIN_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}";
          QML2_IMPORT_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtQmlPrefix}";
        };

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "autonix";
          version = "1.0.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            qt6.wrapQtAppsHook
          ];

          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtwayland
            gtest
            pugixml
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
          ];
        };
      }
    );
}
