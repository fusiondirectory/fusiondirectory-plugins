<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">

 <!-- Headline container -->
 <tr>
  <td colspan="2" style="width:50%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="images/rightarrow.png"/>&nbsp;{t}Generic{/t}</h2>
  </td>
 </tr>
 <tr>
  <td style="vertical-align:top;">
   <table summary="">
    <tr>
     <td><label for="sambaHomePath">{t}Home directory{/t}</label></td>
     <td>
{render acl=$sambaHomePathACL checkbox=$multiple_support checked=$use_sambaHomePath}
      <input type='text' id="sambaHomePath" name="sambaHomePath" size=30 maxlength=60 value="{$sambaHomePath}">
{/render}
{render acl=$sambaHomeDriveACL  checkbox=$multiple_support checked=$use_sambaHomeDrive}
      <select size="1" name="sambaHomeDrive">
       {html_options values=$drives output=$drives selected=$sambaHomeDrive}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="sambaDomainName">{t}Domain{/t}</label></td>
     <td>
{render acl=$sambaDomainNameACL  checkbox=$multiple_support checked=$use_sambaDomainName}
      <select id="sambaDomainName" size="1" name="sambaDomainName"
       onChange="document.mainform.submit();">
       {html_options values=$domains output=$domains selected=$sambaDomainName}
      </select>
{/render}
{render acl=$sambaDomainNameACL  checkbox=$multiple_support checked=$use_sambaDomainName}
     <input type='submit' name='display_information' value='{t}Show information{/t}'>
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;">
   <table summary="">
    <tr>
     <td><label for="">{t}Script path{/t}</label></td>
     <td>
{render acl=$sambaLogonScriptACL  checkbox=$multiple_support checked=$use_sambaLogonScript}
      <input type='text' id="sambaLogonScript" name="sambaLogonScript" size=35 maxlength=60 value="{$sambaLogonScript}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="">{t}Profile path{/t}</label></td>
     <td>
{render acl=$sambaProfilePathACL  checkbox=$multiple_support checked=$use_sambaProfilePath}
      <input type='text' class="center" id="sambaProfilePath" name="sambaProfilePath" size=35 maxlength=60 value="{$sambaProfilePath}">
{/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<h2><img class="center" alt="" align="middle" src="plugins/samba/images/terminal_server.png" />&nbsp;{t}Terminal Server{/t}</h2>

<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>
 <tr>
  <td style="vertical-align:top;width:50%">

{if $multiple_support}
   	<input class="center" type=checkbox name="use_tslogin" id="use_tslogin" value="1" 
		{if $use_tslogin} checked {/if}
		onClick="changeState('tslogin')">
	<input class="center" type=checkbox name="tslogin" id="tslogin" value="1" {$tslogin}
		{if !$use_tslogin} disabled {/if}>
{else}
{render acl=$AllowLoginOnTerminalServerACL}
   <input class="center" type=checkbox name="tslogin" id="tslogin" value="1" {$tslogin}
   	onclick="
		changeState('CtxWFHomeDir');
		changeState('CtxWFHomeDirDrive');
		changeState('CtxWFProfilePath');
		changeState('inherit');			
		changeTripleSelectState_2nd_neg('tslogin','inherit','CtxInitialProgram');
		changeTripleSelectState_2nd_neg('tslogin','inherit','CtxWorkDirectory');
		changeState('CtxMaxConnectionTimeF');
		changeState('CtxMaxDisconnectionTimeF');
		changeState('CtxMaxIdleTimeF');
		changeTripleSelectState('tslogin','CtxMaxConnectionTimeF','CtxMaxConnectionTime');
		changeTripleSelectState('tslogin','CtxMaxDisconnectionTimeF','CtxMaxDisconnectionTime');
		changeTripleSelectState('tslogin','CtxMaxIdleTimeF','CtxMaxIdleTime');
		changeState('connectclientdrives');
		changeState('connectclientprinters');
		changeState('defaultprinter');
		changeState('shadow');
		changeState('brokenconn');
		changeState('reconn');
	">
{/render}
{/if}
   <i>{t}Allow login on terminal server{/t}</i>
   <table summary="">
    <tr>
     <td><label for="CtxWFHomeDir">{t}Home directory{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxWFHomeDir}
      <input type='text' id="CtxWFHomeDir" name="CtxWFHomeDir" size=30 maxlength=60 value="{$CtxWFHomeDir}" {$tsloginstate}>
{/render}
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxWFHomeDirDrive}
      <select size="1" id="CtxWFHomeDirDrive" name="CtxWFHomeDirDrive"  {$tsloginstate}>
       {html_options values=$drives output=$drives selected=$CtxWFHomeDirDrive}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="CtxWFProfilePath">{t}Profile path{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxWFProfilePath}
      <input type='text' id="CtxWFProfilePath" name="CtxWFProfilePath" size=35 maxlength=60 value="{$CtxWFProfilePath}" {$tsloginstate}>
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;">
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_inherit}
   <input class="center" type=checkbox id="inherit" name="inherit" {if $inheritstate} checked {/if}
    {$tsloginstate}
	onClick="changeState('CtxInitialProgram');
 		 changeState('CtxWorkDirectory');"
	
	> 
{/render}
   <i>{t}Inherit client config{/t}</i>
   <table summary="">
    <tr>
     <td><label for="CtxInitialProgram">{t}Initial program{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxInitialProgram}
      <input type='text' id="CtxInitialProgram" name="CtxInitialProgram" size=35 maxlength=60 value="{$CtxInitialProgram}" {$inheritstate} {$tsloginstate}>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="CtxWorkDirectory">{t}Working directory{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxWorkDirectory}
      <input type='text' id="CtxWorkDirectory" name="CtxWorkDirectory" size=35 maxlength=60	value='{$CtxWorkDirectory}' {$inheritstate} {$tsloginstate}>
{/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>
 <tr>
  <td style="vertical-align:top;">
   <i>{t}Timeout settings (in minutes){/t}</i>
   <table summary="">
    <tr>
     <td>
{if $multiple_support}
<input type="checkbox" name="use_CtxMaxConnectionTimeF" {if $use_CtxMaxConnectionTimeF} checked {/if}
	onClick="changeState('CtxMaxConnectionTimeF');" class="center"
	>
{/if}
{render acl=$AllowLoginOnTerminalServerACL}
      <input 		id="CtxMaxConnectionTimeF" 	type="checkbox" class="center" name="CtxMaxConnectionTimeF" 
			{if !$use_CtxMaxConnectionTimeF && $multiple_support} disabled {/if}
			value="1" 			{$CtxMaxConnectionTimeF} 	
			onclick="changeState('CtxMaxConnectionTime')" {$tsloginstate}>
{/render}

      <label for="CtxMaxConnectionTimeF">{t}Connection{/t}</label>
     </td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL}
      <input name="CtxMaxConnectionTime" id="CtxMaxConnectionTime" size=5 maxlength=5 value="{$CtxMaxConnectionTime}" {$tsloginstate}
			{if !$CtxMaxConnectionTimeF} disabled  {/if}>
{/render}
     </td>
    </tr>
    <tr>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxMaxDisconnectionTimeF}
      <input id="CtxMaxDisconnectionTimeF" type=checkbox name="CtxMaxDisconnectionTimeF" value="1" {$CtxMaxDisconnectionTimeF} onclick="changeState('CtxMaxDisconnectionTime')" {$tsloginstate} class="center">
{/render}
      <label for="CtxMaxDisconnectionTimeF">{t}Disconnection{/t}</label>
     </td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL}
      <input name="CtxMaxDisconnectionTime" id="CtxMaxDisconnectionTime" size=5 maxlength=5 value="{$CtxMaxDisconnectionTime}" {$tsloginstate}
			{if !$CtxMaxDisconnectionTimeF} disabled  {/if}>
{/render}
     </td>
    </tr>
    <tr>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_CtxMaxIdleTimeF}
      <input id="CtxMaxIdleTimeF" type=checkbox name="CtxMaxIdleTimeF" value="1" {$CtxMaxIdleTimeF} onclick="changeState('CtxMaxIdleTime')" {$tsloginstate} class="center">
{/render}
      <label for="CtxMaxIdleTimeF">{t}IDLE{/t}</label>
     </td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL}
      <input name="CtxMaxIdleTime" id="CtxMaxIdleTime" size=5 maxlength=5 value="{$CtxMaxIdleTime}" {$tsloginstate}
			{if !$CtxMaxIdleTimeF} disabled  {/if}>
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;">

   <i>{t}Client devices{/t}</i>
   <table summary="">
    <tr>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_connectclientdrives}
      <input id="connectclientdrives" type=checkbox name="connectclientdrives" value="1" {$connectclientdrives} {$tsloginstate} class="center">
{/render}
      <label for="connectclientdrives">{t}Connect client drives at logon{/t}</label>
     </td>
    </tr>
    <tr>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_connectclientprinters}
      <input id="connectclientprinters" type=checkbox name="connectclientprinters" value="1" {$connectclientprinters}{$tsloginstate} class="center">
{/render}
      <label for="connectclientprinters">{t}Connect client printers at logon{/t}</label>
     </td>
    </tr>
    <tr>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_defaultprinter}
      <input id="defaultprinter" type=checkbox name="defaultprinter" value="1" {$defaultprinter} {$tsloginstate} class="center">
{/render}
      <label for="defaultprinter">{t}Default to main client printer{/t}</label>
     </td>
    </tr>
   </table>

  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;width:50%">
   <i>{t}Miscellaneous{/t}</i>
   <table summary="">
    <tr>
     <td>
      <label for="shadow">{t}Shadowing{/t}</label>
     </td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_shadow}
      <select id="shadow" size="1" name="shadow" {$tsloginstate}>
       {html_options options=$shadow selected=$shadowmode}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="brokenconn">{t}On broken or timed out{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_brokenconn}
      <select id="brokenconn" size="1" name="brokenconn" {$tsloginstate}>
       {html_options options=$brokenconn selected=$brokenconnmode}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="reconn">{t}Reconnect if disconnected{/t}</label></td>
     <td>
{render acl=$AllowLoginOnTerminalServerACL  checkbox=$multiple_support checked=$use_reconn}
      <select id="reconn" size="1" name="reconn" {$tsloginstate}>
       {html_options options=$reconn selected=$reconnmode}
      </select>
{/render}
     </td>
    </tr>
   </table>

  </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<h2><img class="center" alt="" align="middle" src="images/lists/key.png" />&nbsp;{t}Access options{/t}</h2>



<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
 <tr>
  <td colspan=2 style="width:50%; vertical-align:top;">

        <!-- Samba policies -->

   <table>
       <tr>
            <td>
                {render acl=$enforcePasswordChangeACL checkbox=$multiple_support checked=$use_enforcePasswordChange}
                 <input type='checkbox' value='1' name='flag_enforcePasswordChange'
                  {if $flag_enforcePasswordChange} checked {/if} id='flag_enforcePasswordChange'>
                {/render}
            </td>
            <td>
                <label for='flag_enforcePasswordChange'>{t}Enforce password change{/t}</label>
            </td>
        </tr>
        <tr>
            <td>
                {render acl=$passwordNeverExpiresACL  checkbox=$multiple_support checked=$use_passwordNeverExpires}
                <input type='checkbox' value='1' name="flag_passwordNeverExpires" 
                    {if $flag_passwordNeverExpires} checked {/if} id='flag_passwordNeverExpires'>
                {/render}
            </td>
            <td>
                <label for="flag_passwordNeverExpires">{t}The password never expires{/t}</label>
            </td>
        </tr>
        <tr>
            <td>
                {render acl=$noPasswordRequiredACL  checkbox=$multiple_support checked=$use_noPasswordRequired}
                <input type='checkbox' value='1' name="flag_noPasswordRequired" 
                    {if $flag_noPasswordRequired} checked {/if} id='flag_noPasswordRequired'>
                {/render}
            </td>
            <td>
                <label for="flag_noPasswordRequired">{t}Login from windows client requires no password{/t}</label>
            </td>
        </tr>
        <tr>
            <td>
                {render acl=$temporaryDisabledACL  checkbox=$multiple_support checked=$use_temporaryDisabled}
                <input type='checkbox' value='1' name="flag_temporaryDisabled" 
                    {if $flag_temporaryDisabled} checked {/if} id='flag_temporaryDisabled'>
                {/render}
            </td>
            <td>
                <label for="flag_temporaryDisabled">{t}Lock samba account{/t}</label>
            </td>
        </tr>
        <tr>
            <td>
                {render acl=$cannotChangePasswordACL  checkbox=$multiple_support checked=$use_cannotChangePassword}
                 <input type='checkbox' value='1' name='flag_cannotChangePassword' 
                    {if $flag_cannotChangePassword} checked {/if} id='flag_cannotChangePassword'>
                {/render}
            </td>
            <td>
                <label for="flag_cannotChangePassword">{t}Cannot change password{/t}</label>
            </td>
        </tr>

	    {if $additional_info_PwdMustChange}
        <tr>
            <td colspan=2 style='padding-left: 10px;'>
	            <i>({$additional_info_PwdMustChange})</i>
            </td>
        </tr>
        {/if}
    </table>
    <table>
      <tr>
        <td colspan=2>
           {render acl=$sambaKickoffTimeACL  checkbox=$multiple_support checked=$use_sambaKickoffTime}
             <input id="flag_sambaKickoffTime" type=checkbox name="flag_sambaKickoffTime" value="1" 
                    {if $flag_sambaKickoffTime} checked {/if} class="center">
           {/render}
           <label for="flag_sambaKickoffTime">{t}Account expires after{/t}</label>
         </td>
         <td style='width:200px;'>
           {render acl=$sambaKickoffTimeACL}
             <input type="text" id="sambaKickoffTime" name="sambaKickoffTime" class="date" style='width:100px' value="{$sambaKickoffTime}">
                {if $sambaKickoffTimeACL|regex_replace:"/[cdmr]/":"" == "w"}
                  <script type="text/javascript">
                    {literal}
                       var datepicker2 = new DatePicker({ 
                            relative : 'sambaKickoffTime', 
                            language : '{/literal}{$lang}{literal}', 
                            keepFieldEmpty : true, 
                            enableCloseEffect : false, 
                            enableShowEffect : false });
                    {/literal}
                  </script>
               {/if}
            {/render}
          </td>
       </tr>
    </table>

    <p class="seperator">&nbsp;</p>
    <br>

   {render acl=$sambaLogonHoursACL mode=read_active  checkbox=$multiple_support checked=$use_SetSambaLogonHours}
     {t}Samba logon times{/t}&nbsp;<input type='submit' name='SetSambaLogonHours' value='{t}Edit settings...{/t}'>
   {/render}
   <!-- /Samba policies -->

  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;">
   <label for="workstation_list">{t}Allow connection from these workstations only{/t}</label>
   <br>

{if $multiple_support}
	<input type="checkbox" name="use_workstation_list" {if $use_workstation_list} checked {/if} class="center"
		onClick="changeState('workstation_list');">
   <select {if $multiple_support && !$use_workstation_list} disabled {/if} 
	id="workstation_list" style="width:100%;" name="workstation_list[]" size=10 multiple>
	
	{foreach from=$multiple_workstations item=item key=key}
		{if $item.UsedByAllUsers}
			<option value="{$key}">{$item.Name} ({t}Used by all users{/t})</option>
		{else}
			<option style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;'
				value="{$key}">{$item.Name} ({t}Used by some users{/t})</option>
		{/if}
	{/foreach}
   </select>
   <br>
   <input type="submit" value="{msgPool type=addButton}" name="add_ws">
   <input type="submit" value="{msgPool type=delButton}" name="delete_ws">
{else}
	{render acl=$sambaUserWorkstationsACL}
	   <select id="workstation_list" style="width:100%;" name="workstation_list[]" size=10 multiple>
		{html_options values=$workstations output=$workstations}
	   </select>
	{/render}
	   <br>
	{render acl=$sambaUserWorkstationsACL}
	   <input type="submit" value="{msgPool type=addButton}" name="add_ws">
	{/render}
	{render acl=$sambaUserWorkstationsACL}
	   <input type="submit" value="{msgPool type=delButton}" name="delete_ws">
	{/render}
{/if}
  </td>
 </tr>
</table>

<input type="hidden" name="sambaTab" value="sambaTab">
