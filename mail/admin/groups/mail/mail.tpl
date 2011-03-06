<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">
 <tr>
  <td style="width:50%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="images/rightarrow.png" />&nbsp;{t}Generic{/t}</h2>
   <table summary="">
    <tr>
     <td><label for="mail">{t}Primary address{/t}</label>{$must}</td>
     <td>
	 {if !$isModifyableMail && $initially_was_account}
		<input type='text' disabled size=30 value="{$mail}">
	 {else}
		 {if $domainSelectionEnabled}
			{render acl=$mailACL}
				<input id="mail" name="mail" size=20 maxlength=65 value="{$mail}"
					{if $mailEqualsCN} disabled {/if}
				>
			{/render}
			@<select name='MailDomain'>
				{html_options values=$MailDomains output=$MailDomains selected=$MailDomain}
			</select>
		{else}
			{if $mailEqualsCN}
				<input type='text' disabled name='dummy' value='{$mail}'>
				@<input type='text' value="{$MailDomain}" name="MailDomain">
			{else}
			{render acl=$mailACL}
				<input type='text' id="mail" name="mail" size=35 maxlength=65 value="{$mail}">
			{/render}
			{/if}
		{/if}
    {/if}
     </td>
    </tr>
    <tr>
     <td><label for="gosaMailServer">{t}Server{/t}</label></td>
     <td>
     {if !$isModifyableServer && $initially_was_account}
        <input type='text' disabled size=30 value="{$gosaMailServer}">
     {else}
{render acl=$gosaMailServerACL}
      <select size="1" id="gosaMailServer" name="gosaMailServer" 
		title="{t}Specify the mail server where the user will be hosted on{/t}">
        {html_options values=$MailServers output=$MailServers selected=$gosaMailServer}
        <option disabled>&nbsp;</option>
      </select>
{/render}
     {/if}
     </td>
    </tr>
    <tr>
     <td>&nbsp;
     </td>
    </tr>
{if $quotaEnabled}
    <tr>
     <td>{t}Quota usage{/t}</td>
     <td>{$quotaUsage}</td>
    </tr>
    <tr>
     <td><label for="gosaMailQuota">{t}Quota size{/t}</label></td>
     <td>
{render acl=$gosaMailQuotaACL}
      <input id="gosaMailQuota" name="gosaMailQuota" size="6" align="middle" maxlength="60"
        value="{$gosaMailQuota}"> MB
            {/render}
     </td>
    </tr>
{/if}

{if $folderTypesEnabled && !$multiple_support}
	<tr>
		<td>
			{t}Folder type{/t}
		</td>
		<td>
			<select id="FolderTypeCAT" name="FolderTypeCAT" onChange="document.mainform.submit();">
				{foreach from=$AvailableFolderTypes.CAT item=item key=key}
					<option {if $key == $FolderType.CAT} selected {/if} value="{$key}">{$item}</option>
				{/foreach}
			</select>
			<select id="FolderTypeSUB_CAT" name="FolderTypeSUB_CAT" onChange="document.mainform.submit();">
				{foreach from=$AvailableFolderTypes.SUB_CAT item=item key=key}
                    {if $key == $FolderType.CAT} 
						{foreach from=$item item=item2 key=key2}
							<option {if $key2 == $FolderType.SUB_CAT} selected {/if}
								value='{$key2}'>{$item2}</option>
						{/foreach}
					{/if}
				{/foreach}
			</select>
			<input type='image' src='images/lists/reload.png' class='center' alt='{t}Reload{/t}'>
		</td>
	</tr>
	{/if}
   </table>
  </td>

<!-- Alternate addresses -->
{if !$multiple_support}
  <td style="vertical-align:top;padding-left:2px;">
   <h2><img class="center" alt="" align="middle" src="plugins/mail/images/alternatemail.png"> 
	{t}Alternative addresses{/t}
   </h2>

{render acl=$gosaMailAlternateAddressACL}
   <select style="width:100%;" name="alternates_list[]" size=10 multiple 
	title="{t}List of alternative mail addresses{/t}">
    {html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
	<option disabled>&nbsp;</option>
   </select>
{/render}
   <br>
{render acl=$gosaMailAlternateAddressACL}
   <input type='text' name="alternate_address" size="30" align=middle maxlength="60" value="">
{/render}

{render acl=$gosaMailAlternateAddressACL}
   <input type=submit value="{msgPool type=addButton}" name="add_alternate">&nbsp;
{/render}

{render acl=$gosaMailAlternateAddressACL}
   <input type=submit value="{msgPool type=delButton}" name="delete_alternate">
{/render}
  </td>
{/if}
 </tr>
</table>

<p class="seperator">&nbsp;</p>

{if !$multiple_support}
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
 <tr>
  <td style="vertical-align:top;width:50%; border-right:1px solid #A0A0A0">
   <h2><img class="center" alt="" align="middle" src="plugins/mail/images/shared_folder.png"> 
		{t}IMAP shared folders{/t}
   </h2>
   <input type='hidden' name='mail_acls_posted' value='1'>
   <table summary="" cellpadding=0 border=0>
	{foreach from=$folder_acls item=item key=user}
		<tr>
		{if $user == "__anyone__"}
     		<td><LABEL for="default_permissions">{t}Default permission{/t}</LABEL></td>
		{elseif $user == "__member__"}
     		<td><LABEL for="member_permissions">{t}Member permission{/t}</LABEL></td>
		{else}
     		<td>
				<input type='input' name='acl_user_{$item.post_name}' value='{$user}'>
			</td>
		{/if}
		 <td>
{render acl=$aclACL}
		  <select size="1" name="acl_value_{$item.post_name}">
		   {html_options options=$AclTypes selected=$item.acl}
		   <option disabled>&nbsp;</option>
		  </select>
			{if !($user == "__anyone__" || $user == "__member__")}
		  		<input type='submit' value='{msgPool type=delButton}' name='remove_acl_user_{$item.post_name}'>
			{/if}
{/render}
			{if $user == "__member__"}
				{if $show_effective_memeber}
					<input type='submit' name='show_effective_memeber' value='{t}Hide{/t}'> 
				{else}
					<input type='submit' name='show_effective_memeber' value='{t}Show{/t}'> 
				{/if}
			{/if}
		 </td>
		</tr>
		{if $user == "__member__" && $show_effective_memeber}
			{foreach from=$Effective item=i key=k}
				<tr><td>&nbsp;&nbsp;<i>{$k}</i></td></tr>
			{/foreach}
		{/if}
	{/foreach}

		
		<tr>
			<td colspan="1"></td>
			<td><input type='submit' value='{msgPool type=addButton}' name='add_acl_user'>
		</tr>
   </table>
{/if}      

<p class="seperator">&nbsp;</p>

<h2><img class="center" alt="" align="middle" src="images/false.png" />&nbsp;{t}Advanced mail options{/t}</h2> 
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="2" border="0">
 <tr>
  <td>
{render acl=$gosaMailDeliveryModeIACL}
   <input type="checkbox" name="only_local" value="1" {$only_local} 
	title="{t}Select if user can only send and receive inside his own domain{/t}">
{/render}
	{t}User is only allowed to send and receive local mails{/t}
  </td> 
 </tr>
 </table> 

  </td>
  <td style="vertical-align:top;width:50%">
   <h2>
	<img class="center" alt="" align="middle" src="plugins/mail/images/envelope.png">
	{t}Forward messages to non group members{/t}
   </h2>

{render acl=$gosaMailForwardingAddressACL}
   <select style="width:100%;" name="forwarder_list[]" size=10 multiple>

	{if $multiple_support}

		{foreach from=$Forward_all item=item key=key}
			<option value="{$item}">{$item}&nbsp;({t}Used in all groups{/t})</option>
		{/foreach}
		{foreach from=$Forward_some item=item key=key}
			<option value="{$item}" style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;'>{$item}&nbsp;({t}Not used in all groups{/t})</option>
		{/foreach}
	{else}
    {html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
	<option disabled>&nbsp;</option>
	{/if}
   </select>
{/render}

   <br>

{render acl=$gosaMailForwardingAddressACL}
   <input type='text' name="forward_address" size=20 align=middle maxlength=65 value="">
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{msgPool type=addButton}" name="add_forwarder">&nbsp;
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{t}Add local{/t}" name="add_local_forwarder">&nbsp;
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{msgPool type=delButton}" name="delete_forwarder">
{/render}

  </td>
 </tr>
</table>
<input type="hidden" name='mailedit' value='1'>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('mail');
  -->
</script>
{if $multiple_support}
	<input type="hidden" name="multiple_mail_group_posted" value="1">
{/if}
