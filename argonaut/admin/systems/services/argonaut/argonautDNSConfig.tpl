<br/>
{render acl=$argonautDNSConfigACL}
<table>
<tr><td colspan="2"><br/><h3>Argonaut Ldap2zone settings:</h3></td></tr>
{foreach from=$attributes key=key item=value name=loop}
<tr>
  <td><label for="{$key}">
  {if $key=='argonautLdap2zoneBindDir'}
    {t}Bind directory{/t}
  {elseif $key=='argonautLdap2zoneAllowNotify'}
    {t}Allow notify{/t}
  {elseif $key=='argonautLdap2zoneAllowUpdate'}
    {t}Allow update (semicolon separated and ended){/t}
  {elseif $key=='argonautLdap2zoneAllowTransfer'}
    {t}Allow transfer (semicolon separated and ended){/t}
  {elseif $key=='argonautLdap2zoneTTL'}
    {t}TTL{/t}
  {elseif $key=='argonautLdap2zoneRndc'}
    {t}rndc path{/t}
  {/if}
  </label></td>
  <td><input id="{$key}" name="{$key}"
  {if in_array($key,$booleans)}
    type="checkbox"
    {if $value==1} checked="checked"{/if}
  {else}
    type="text"
    value="{$value}"
  {/if}
  /></td>
</tr>
{/foreach}
</table>
{/render}

<p class="seperator">&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
  <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
  &nbsp;
  <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="argonautDNSConfigPosted" value="1">
