{
  description = "DevShell for ORB_SLAM3 CMake project";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };
        opencvgtk = pkgs.opencv.override {
          enableGtk2 = true;
          enableGtk3 = true;
        };
      in {
        devShells.default = pkgs.mkShell {
          name = "orb_slam3-devshell";

          buildInputs = with pkgs; [
            gcc
            pkg-config
            eigen
            # sophus
            # g2o
            opencvgtk
            # gtk3
            pangolin
            glew
            librealsense
            boost
            openssl
          ];
          nativeBuildInputs = [ pkgs.cmake ];
          shellHook = let icon = "f121";
          in ''
            echo "Welcome to the ORB_SLAM3 development shell!"
            export CMAKE_PREFIX_PATH=${pkgs.eigen}:${pkgs.opencv}:${pkgs.pangolin}:${pkgs.librealsense}:${pkgs.g2o}:${pkgs.sophus}:${pkgs.glew}
            export G2O_PATH=${pkgs.g2o}
            export SOPHUS_PATH=${pkgs.sophus}
          '';
        };
      });
}
