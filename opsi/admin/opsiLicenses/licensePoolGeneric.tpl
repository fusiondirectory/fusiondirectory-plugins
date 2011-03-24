{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if GOto-si is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}Retry{/t}">
<br>
<br>
{else}


<table width="100%">
  <tr> 
    <td style='vertical-align:top;'>
        <!-- GENERIC -->
        <h2>{t}Generic{/t}</h2>
        <table>
          <tr> 
            <td>{t}Name{/t}</td>
            <td>
              {if $initially_was_account}
                <input type='text' value='{$cn}' disabled>
              {else}
{render acl=$cnACL}
              <input type='text' value='{$cn}' name='cn'>
{/render}
              {/if}
            </td>
          </tr>
          <tr> 
            <td>{t}Description{/t}</td>
            <td>
{render acl=$descriptionACL}
              <input type='text' value='{$description}' name='description'>
{/render}
            </td>
          </tr>
        </table>

    </td>
    <td style='width:50%; border-left: 1px solid #AAA;padding: 5px;'>
        <!-- LICENSES -->
        <h2>{t}Licenses{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
              {$licenses}
{render acl=$licensesACL}
              <input type='submit' name='addLicense' value='{msgPool type=addButton}'>
{/render}
            </td>
          </tr>
        </table>

    </td>
  </tr>
  <tr> 
    <td colspan="2">
      <p class='separator'>&nbsp;</p>
    </td>
  </tr>
  <tr>
    <td style='width:50%'>
        <!-- APPLICATIONS -->
        <h2>{t}Applications{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
{render acl=$productIdsACL}
              <select name='productIds[]' multiple size="6" style="width:100%;">
                {html_options options=$productIds}
              </select><br>
{/render}
{render acl=$productIdsACL}
              <select name='availableProduct'>
                {html_options options=$availableProductIds}
              </select>
{/render}
{render acl=$productIdsACL}
              <input type='submit' name='addProduct' value='{msgPool type='addButton'}'>
{/render}
{render acl=$productIdsACL}
              <input type='submit' name='removeProduct' value='{msgPool type='delButton'}'>
{/render}
            </td>
          </tr>
        </table>

    </td>
    <td style="border-left: 1px solid #AAA; padding: 5px;vertical-align:top">
        <!-- SOFTWARE -->
        <h2>{t}Windows software IDs{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
{render acl=$windowsSoftwareIdsACL}
              <select name='softwareIds[]' multiple size="6" style="width:100%;">
                {html_options options=$softwareIds}
              </select>
{/render}
<!--
{render acl=$windowsSoftwareIdsACL}
              <input type='text' name='newSoftwareId' value='' size=10>
{/render}
{render acl=$windowsSoftwareIdsACL}
              <input type='submit' name='addSoftware' value='{msgPool type='addButton'}'>
{/render}
{render acl=$windowsSoftwareIdsACL}
              <input type='submit' name='removeSoftware' value='{msgPool type='delButton'}'>
{/render}
-->
            </td>
          </tr>
        </table>

    </td>
  </tr>
</table>
<input name='opsiLicensePoolPosted' value='1' type='hidden'>
{/if}
