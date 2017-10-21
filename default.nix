{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation rec {
  name = "et";

  src = ./.;

  buildInputs = with pkgs; [ libnotify gdk_pixbuf ];
  nativeBuildInputs = [ pkgs.pkgconfig ];

  installPhase = ''
    mkdir -p $out/bin
    cp et $out/bin
    cp et-status.sh $out/bin/et-status
  '';
}
