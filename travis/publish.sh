git clone https://github.com/vicr123/vicr123.github.io.git MainSite
cd MainSite/repo/arch/x86_64
git rm theshell-blueprint*.pkg.tar.xz
cp /build/theshell-blueprint/theshell-blueprint*.pkg.tar.xz ./
repo-add theappsbp.db.tar.gz theshell-blueprint*.pkg.tar.xz
git add .
git push https://vicr123:$GITHUB_TOKEN@github.com/vicr123/vicr123.github.io.git
