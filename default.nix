{ stdenv }:

stdenv.mkDerivation rec {
  name = "petool";
  version = "1.1.1.1";

  buildInputs = [ ];
  src = ./.;
  preBuild = "makeFlagsArray=(REV=${version})";

  installPhase = ''
    mkdir -p $out/bin
    install -m 755 petool $out/bin
  '';

  enableParallelBuilding = true;

  meta = with stdenv.lib; {
    homepage = http://github.com/FunkyFr3sh/petool;
    description = "Tool to help rebuild, extend and patch 32-bit Windows applications.";
    # maintainers =
    license = map (builtins.getAttr "shortName") [ licenses.mit ];
    # Should be portable C
    platforms = stdenv.lib.platforms.all;
  };
}
