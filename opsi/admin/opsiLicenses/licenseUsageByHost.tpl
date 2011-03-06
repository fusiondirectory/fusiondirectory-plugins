{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}Retry{/t}">
<br>
<br>
{else}


<table width="100%">
  <tr>
    <td style='vertical-align:top;width: 50%; padding-right:5px; border-right: solid 1px #AAA; '>
        <h2>{t}Reserved for{/t}</h2>
{render acl=$boundToHostACL}
        {$licenseReserved}
{/render}
{render acl=$boundToHostACL}
        <select name='availableLicense'>
{/render}
          {html_options options=$availableLicenses}
        </select>
{render acl=$boundToHostACL}
        <input type='submit' name='addReservation' value='{msgPool type=addButton}'>
{/render}
    </td>
    <td style='vertical-align:top;'>
        <h2>{t}Licenses used{/t}</h2>
{render acl=$boundToHostACL}
        {$licenseUses}
{/render}
    </td>
  </tr>
</table>

<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}
