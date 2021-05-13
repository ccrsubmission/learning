{ pkgs ? import <nixpkgs> {}}:
pkgs.ns2.overrideAttrs (oldAttrs: {
  nativeBuildInputs = with pkgs; oldAttrs.nativeBuildInputs ++ [ 
      pkgconfig nlohmann_json boost otcl tclcl cmake tcl tk
  ];
  shellHook = ''
    export CMAKE_INCLUDE_PATH="$CMAKE_INCLUDE_PATH:$PWD/../self-adapting-simulator/src/schad_common" 
    export CMAKE_INCLUDE_PATH="$CMAKE_INCLUDE_PATH:$PWD/../self-adapting-simulator/src/schad_learning" 
    export CMAKE_LIBRARY_PATH="$CMAKE_LIBRARY_PATH:$PWD/../self-adapting-simulator/cmake-build-release/src/schad_learning" 
  '';
})
