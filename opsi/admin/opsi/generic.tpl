
<h2><img src='plugins/opsi/images/client_generic.png' alt=' ' class='center'>&nbsp;{t}OPSI host{/t}</h2>

{if $init_failed}

<font style='color: #FF0000;'>{msgPool type=siError p=$message}</font>

<input type='submit' name='reinit' value="{t}Retry{/t}">

{else}

<table style="width: 100%;">
 <tr>
  <td>
   <table>
    {if $standalone}
    <tr>
     <td>{t}Name{/t}{$must}</td>
     <td>
{render acl=$hostIdACL}
		<input style='width:300px;' type='text' name='hostId' value='{$hostId}'>
{/render}
	 </td>
    </tr>
<!--
    <tr>
     <td>{t}MAC address{/t}{$must}</td>
     <td>
{render acl=$macACL}
		<input type='text' name="dummy" value="{$mac}" disabled>
{/render}
	 </td>
	</tr>
-->
	{else}
    <tr>
     <td>{t}Name{/t}</td>
     <td>
{render acl=$hostIdACL}
		<input style='width:300px;' type='text' disabled value="{$hostId}">
{/render}
	 </td>
    </tr>
<!--
    <tr>
     <td>{t}MAC address{/t}{$must}</td>
     <td>
{render acl=$macACL}
		<input type='text' name="mac" value="{$mac}">
{/render}
	 </td>
    </tr>
-->
    {/if}
    <tr>
     <td>{t}Netboot product{/t}</td>
     <td>
{render acl=$netbootProductACL}
      <select name="opsi_netboot_product" onChange="document.mainform.submit();">
		{foreach from=$ANP item=item key=key}
			<option {if $key == $SNP} selected {/if} value="{$key}">{$key}</option>
		{/foreach}
      </select>
{/render}
      &nbsp;
      {if $netboot_configurable}
		  <input type='image' name='configure_netboot' src='images/lists/edit.png'
			title='{t}Configure product{/t}' class='center'>
      {else}
<!--		  <input type='image' name='dummy_10' src='images/lists/edit_gray.png'
			title='{t}Configure product{/t}' class='center'>-->
      {/if}
     </td>
    </tr>
   </table>
  </td>
  <td style='vertical-align: top;'>
   <table>
    <tr>
     <td>{t}Description{/t}</td>
     <td>
{render acl=$descriptionACL}
		<input type='text' name='description' value='{$description}'>
{/render}
	 </td>
    </tr>
    <tr>
     <td>{t}Notes{/t}</td>
     <td>
{render acl=$descriptionACL}
		<input type='text' name='note' value='{$note}'>
{/render}
	 </td>
    </tr>
   </table>
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <p class='seperator'>&nbsp;</p>
  </td>
 </tr>
 <tr>
  <td style="width:50%;"><h2><img class='center' src='plugins/opsi/images/product.png' 
		alt=' '>&nbsp;{t}Installed products{/t}</h2>
{render acl=$localProductACL}
	{$divSLP}
{/render}
  </td>
  <td style="width:50%;"><h2>{t}Available products{/t}</h2>
{render acl=$localProductACL}
	{$divALP}
{/render}
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <p class='seperator'>&nbsp;</p><br>
   {if $standalone}
    <h2><img src='images/rocket.png' alt="" class="center">&nbsp;{t}Action{/t}</h2>
	<select name='opsi_action'>
		<option>&nbsp;</option>
		{if $is_installed}
		<option value="install">{t}Reinstall{/t}</option>
		{else}
		<option value="install">{t}Install{/t}</option>
		{/if}
		<option value="wake">{t}Wake{/t}</option>
	</select>
{render acl=$triggerActionACL}
	<input type='submit' name='opsi_trigger_action' value="{t}Execute{/t}">
{/render}
   {/if}
  </td>
 </tr>
</table> 
<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>

{$netconfig}

<input type='hidden' name='opsiGeneric_posted' value='1'>
{/if}
