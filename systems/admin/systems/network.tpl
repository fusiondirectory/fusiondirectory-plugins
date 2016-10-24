<h2><img  class="center"  alt=""  align="middle"  src="geticon.php?context=categories&icon=applications-internet&size=16"> {t}Network  settings{/t}</h2>


<table style="width:100%; border-spacing:0; border-collapse:collapse;">
  <tr>
    <td style="width:50%; vertical-align: top;">
      <table>
        <tr>
          <td style='vertical-align:top;'><LABEL  for="ipHostNumber">{t}IP-address{/t}{if $IPisMust}{$must}{/if}</LABEL></td>
          <td>
{render acl=$ipHostNumberACL}
            <input  type='text' id="ipHostNumber" name="ipHostNumber" size=25 maxlength=80  value="{$ipHostNumber}">
{/render}
          {foreach from=$additionalHostNumbers item=item key=key}
            <br>
{render acl=$ipHostNumberACL}
            <input size=25 maxlength=80 type='text' name='additionalHostNumbers_{$key}' value='{$item}'>
{/render}
{render acl=$ipHostNumberACL}
            <input type='image' class='center' name='additionalHostNumbers_del_{$key}' src='geticon.php?context=actions&icon=edit-delete&size=16' alt='{msgPool type=delButton}'>
{/render}
          {/foreach}
{render acl=$ipHostNumberACL}
          <input type='image' class='center' name='additionalHostNumbers_add}' src='geticon.php?context=actions&icon=document-new&size=16' alt='{msgPool type=addButton}'>&nbsp;
{/render}
<br>

{render acl=$ipHostNumberACL}
      <input id="propose_ip" type="submit" name="propose_ip" value="{t}Propose ip{/t}" style="display: none;">
{/render}
          </td>
        </tr>
        <tr>
          <td><LABEL  for="macAddress">{t}MAC-address{/t}</LABEL>{if $MACisMust}{$must}{/if}</td>
          <td>
{render acl=$macAddressACL}
            <input  type='text' name="macAddress" id="macAddress" size=25 maxlength=80  value="{$macAddress}">
{/render}
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>
<input type="hidden" name="network_tpl_posted" value="1">

<!--
vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
-->
