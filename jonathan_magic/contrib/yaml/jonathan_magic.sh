#!/usr/bin/env bash
while read -r line; do
  if grep -q 'Package' <<< "$line" ; then
    package="$(echo "${line}" | cut -d ":" -f2 | sed 's/^\ //g')"
  fi

  if grep -q 'Description' <<< "$line" ; then
    description="$(echo "${line}" | cut -d ":" -f2 | sed 's/^\ //g')"
  fi

#echo "P: $package"
#echo "D: ${description^}"

cat << EOF > "$package".yaml
information:
  name : $package
  description : ${description^}
  version : "1.4"
  authors :
    - "FusionDirectory"
  status : Stable
  screenshotUrl:
    - "%to_be_define%"
  logoUrl : "https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins/-/raw/fusiondirectory-1.4/$package/html/themes/breezy/icons/48/apps/$package.png"
  # https://gitlab.fusiondirectory.org/applications/demonstration/-/blob/bulleyes-selenium/demoInstall.sh
  tags: ["plugin", "$package"]
  license: "GPLv2"
  origin: "package"

support:
  provider: fusiondirectory
  homeUrl : https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins
  ticketUrl : https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins/-/issues
  schemaUrl: "https://schemas.fusiondirectory.org/"
  contractUrl: https://www.fusiondirectory.org/abonnements-fusiondirectory/

requirement:
  fdVersion : 1.4
  phpVersion : 7.3.0
EOF

done < plugins_description.yaml
