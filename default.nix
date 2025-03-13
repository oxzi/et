with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "et";

  src = lib.cleanSource ./.;

  buildInputs = [ libnotify gdk-pixbuf ];
  nativeBuildInputs = [ pkg-config ];

  installPhase = ''
    mkdir -p $out/bin
    cp et $out/bin
    cp et-status.sh $out/bin/et-status
  '';
}
