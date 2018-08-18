for i in /build/theshell-blueprint/theshell-blueprint*.pkg.tar.xz; do
    curl -X POST --data-binary @$i -H "Filename: $(basename $i)" -H "Repository: theappsbp" "https://$REPO_USERNAME:$REPO_PASSWORD@packages.vicr123.com/push"
done
