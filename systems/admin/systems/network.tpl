<h2><img	class="center"	alt=""	align="middle"	src="plugins/systems/images/network.png">	{t}Network	settings{/t}</h2>


<table	summary=""	width="100%" cellspacing=0 cellpadding=0>
	<tr>
		<td	style="width:50%; vertical-align: top;">
			<table	summary="">
				<tr>
					<td style='vertical-align:top;'><LABEL	for="ipHostNumber">{t}IP-address{/t}{if $IPisMust}{$must}{/if}</LABEL></td>
					<td>
{render acl=$ipHostNumberACL}
            <input	type='text' id="ipHostNumber"	name="ipHostNumber"	size=25	maxlength=80	value="{$ipHostNumber}">
{/render}
          {foreach from=$additionalHostNumbers item=item key=key}
            <br>
{render acl=$ipHostNumberACL}
            <input size=25 maxlength=80 type='text' name='additionalHostNumbers_{$key}' value='{$item}'>
{/render}
{render acl=$ipHostNumberACL}
            <input type='image' class='center' name='additionalHostNumbers_del_{$key}' src='images/lists/trash.png' alt='{msgPool type=delButton}'>
{/render}
          {/foreach}
{render acl=$ipHostNumberACL}
          <input type='image' class='center' name='additionalHostNumbers_add}' src='images/lists/new.png' alt='{msgPool type=addButton}'>&nbsp;
{/render}
<br> 

{render acl=$ipHostNumberACL}
{if $DNS_is_account == true}
      <input id="propose_ip" type="submit" name="propose_ip" value="{t}Propose ip{/t}">
      {else}
      <input id="propose_ip" type="submit" name="propose_ip" value="{t}Propose ip{/t}" style="display: none;">
      {/if}
{/render}
          </td>
				</tr>
				<tr>
					<td><LABEL	for="macAddress">{t}MAC-address{/t}</LABEL>{if $MACisMust}{$must}{/if}</td>
					<td>
{render acl=$macAddressACL}
            <input	type='text' name="macAddress"	id="macAddress"	size=25	maxlength=80	value="{$macAddress}">
{/render}

{render acl=$autonetACL}
      	    <input type="submit" name="autonet" value="{t}Autodetect{/t}">
{/render}
          </td>
				</tr>
        {if $dhcpEnabled}
        <tr>
          <td colspan=2 style='padding-top:12px;'>
            <table>
              {if $dhcpParentNodeCnt}
              <tr>
                <td>
{render acl=$dhcpSetupACL}
                  <input onClick='document.mainform.submit();'
                    {if $dhcp_is_Account} checked {/if} type='checkbox' name='dhcp_is_Account' class='center'>
{/render}
                </td>
                <td colspan="2">
                  {t}Enable DHCP for this device{/t}
{render acl=$dhcpSetupACL}
                  <input type='image' src='images/lists/reload.png' class='center'>
{/render}
                </td>
              </tr>
              {else}
              <tr>
                <td>
                  <input type='checkbox' name='dummy' class='center' disabled>
                  {t}Enable DHCP for this device{/t} ({t}not configured{/t})
                  <input type='image' src='images/lists/reload.png' class='center'>
                </td>
              </tr>
              {/if}
              {if $dhcp_is_Account}
              <tr>
                <td>&nbsp;</td>
                <td>{t}Parent node{/t}</td>            
                <td>
{render acl=$dhcpSetupACL}
                  <select name='dhcpParentNode'>      
                    {html_options options=$dhcpParentNodes selected=$dhcpParentNode}
                  </select>
{/render}
                </td>
              </tr>
              <tr>
                <td>&nbsp;</td>
                <td>&nbsp;</td>
                <td>
                  <input type='submit' name='dhcpEditOptions' value='{t}Edit settings{/t}'>
                </td>
              </tr>
              {/if}
            </table>
          </td>
        </tr>
        {/if}
      </table> 
    </td>
    {if $DNSenabled==true}
		<td	style="width:50%;vertical-align:top;border-left:1px solid #b0b0b0;" valign="top">

    {if $ZoneCnt}

        {render acl=$dnsSetupACL}
            {if $DNS_is_account == true}

                <input type="checkbox" name="DNS_is_account" id="DNS_is_account" value="1" checked="checked" 
                    {if $hide_dns_check_box}disabled
                    {else}onclick="$('test2').toggle(); $('propose_ip').toggle();"{/if}
                />

            {else}
                <input type="checkbox" name="DNS_is_account" id="DNS_is_account" value="1"
                    onclick="
                    $('test2').toggle();
                    $('propose_ip').toggle();
                "/>
            {/if}
        {/render}

      <label for="DNS_is_account">{t}Enable DNS for this device{/t}</label>
      <input type='image' src='images/lists/reload.png' class='center' name="reloadThisDNSStuff" >
      {if $DNS_is_account == true}
      <div style="padding-left:20px" id="test2">
      {else}
      <div style="padding-left:20px;display: none;" id="test2">
      {/if}
			<table summary="">
				<tr>
					<td><LABEL	for="zoneName">{t}Zone{/t}</LABEL></td>
					<td>
            {render acl=$dnsSetupACL}
              <select name="zoneName" id="zoneName">
                {html_options values=$ZoneKeys output=$Zones selected=$zoneName}
              </select>
            {/render}
          </td>
        </tr>
        <tr>
          <td><label for ="dNSTTL">{t}TTL{/t}</label></td>
          <td>
            {render acl=$dnsSetupACL}
                        <input type="text" name="dNSTTL" value="{$dNSTTL}" id="dNSTTL" >
            {/render}
          </td>
        </tr>
        <tr>
          <td valign="top">{t}Dns records{/t}</td>
          <td>
            {render acl=$dnsSetupACL}
                          {$records}
            {/render}
          </td>
				</tr>
			</table>
      </div>
    {else}        
      <input type="checkbox" name="dummy" id="DNS_is_account" value="1" disabled class='center' {if $DNS_is_account} checked {/if}>
      <label for="DNS_is_account">{t}Enable DNS for this device{/t} ({t}not configured{/t})</label>
      <input type='image' src='images/lists/reload.png' class='center'>
    {/if}

		</td>
    {/if}
    
	</tr>
</table>
<input type="hidden" name="network_tpl_posted" value="1">

<!--
vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
-->
