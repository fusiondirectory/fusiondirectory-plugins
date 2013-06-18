<table width="100%">
  <tr>
    <td width="50%" valign="top" style="border-right:1px solid #A0A0A0">
        <h2><img class="center" alt="" src="images/forward.png" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
        <table cellspacing="4">
          <tr>
            <td>
              <LABEL for="cn">
              {t}Name{/t}
              </LABEL>
            </td>
            <td>
{render acl=$cnACL}
              <input type='text' value="{$cn}" size="45" id="cn" name="cn"/>
{/render}
            </td>
          </tr>
          <tr>
            <td>
              <LABEL for="description">
              {t}Description{/t}
              </LABEL>
            </td>
            <td>
{render acl=$descriptionACL}
              <input type='text' value="{$description}" size="45" name="description" id="description">
{/render}
            </td>
          </tr>
        </table>
    </td>
    <td width="50%" valign="top">
      <h2><img class="center" alt="" src="plugins/fai/images/repository.png" title="{t}Repository{/t}">&nbsp;{t}Repository{/t}</h2>
        <table cellspacing="4">
          <tr>
            <td>
              {t}Release{/t} :
            </td>
            <td>
              {$release}
            </td>
          </tr>
          <tr>
            <td>
              {t}Section{/t} :
            </td>
            <td>
              {$section}
            </td>
          </tr>
          <tr>
              <td>
              {t}Install method{/t} :
            </td>
            <td>
{render acl=$FAIinstallMethodACL}
              <select name="FAIinstallMethod" title='{t}Please select the installation method{/t}'>
                {html_options options=$FAIinstallMethods output=$FAIinstallMethod selected=$FAIinstallMethod}
              </select>
{/render}
            </td>
          </tr>
        </table>
    </td>
  </tr>
</table>
<p class="seperator">&nbsp;</p>
<table width="99%">
  <tr>
    <td>
      <h2><img class="center" alt="" src="plugins/fai/images/fai_packages.png" title="{t}Used packages{/t}">&nbsp;{t}Used packages{/t}</h2>
      {$divlist}
    </td>
  </tr>
{render acl=$FAIpackageACL}
  <tr>
    <td>
            <input type="text" size="25" name="addPpkgsText" value="" />
      <input type="submit" name="AddManualpkg" value="{msgPool type=addButton}" />
      &nbsp;
      <input type="submit" name="Addpkg" value="{t}Add from list{/t}" />
    </td>
  </tr>
{/render}
</table>

<!-- Place cursor -->
<script type="text/javascript">
  <!-- // First input field on page
  focus_field('cn','description');
  -->
</script>

