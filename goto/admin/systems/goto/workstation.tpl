<h2><img class="center" alt="" align="middle" src="images/rightarrow.png"> {t}Properties{/t}</h2>
<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top;">
   <table summary="">
    <tr>
{if $cn eq 'wdefault'}
     <td colspan=2>{t}Workstation template{/t}</td>
{else}
     <td><LABEL for="cn">{t}Workstation name{/t}</LABEL>{$must}</td>
     <td>
{render acl=$cnACL}
      <input type='text' name="cn" id="cn" size=18 maxlength=60 value="{$cn}">
{/render}
     </td>
{/if}
    </tr>
    <tr>
     <td><LABEL for="description">{t}Description{/t}</LABEL></td>
     <td>
{render acl=$descriptionACL}
      <input type='text' name="description" id="description" size=18 maxlength=60 value="{$description}">
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="cn">{t}Location{/t}</LABEL></td>
     <td>
{render acl=$lACL}
      <input type='text' name="l" id="l" size=18 maxlength=60 value="{$l}">
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="base">{t}Base{/t}</LABEL>{$must}</td>
     <td>
{render acl=$baseACL}
       {$base}
{/render}
     </td>
    </tr>
   </table>

	{$host_key}

  </td>
  <td  style="vertical-align:top;border-left:1px solid #A0A0A0;">
   <table summary="">
    <tr>
     <td>{t}Mode{/t}</td>
     <td>
{render acl=$gotoModeACL}
      <select name="gotoMode" title="{t}Select terminal mode{/t}">
       {html_options options=$modes selected=$gotoMode_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoSyslogServer">{t}Syslog server{/t}</LABEL></td>
     <td>
{render acl=$gotoSyslogServerACL}
      <select id="gotoSyslogServer" name="gotoSyslogServer" title="{t}Choose server to use for logging{/t}">
       {html_options options=$syslogservers selected=$gotoSyslogServer_select}
      </select>
{/render}
     </td>
    </tr>
   </table>
<p class="seperator">&nbsp;</p>
   <table width="100%">
    <tr>
     <td colspan="2">
{if $member_of_ogroup}
{render acl=$gotoNtpServerACL}
        <input type="checkbox" value="1" name="inheritTimeServer"
            {if $inheritTimeServer} checked {/if}
            onClick="javascript:
                    changeState('gotoNtpServerSelected');
                    changeState('gotoNtpServers');
                    changeState('addNtpServer');
                    changeState('delNtpServer');">{t}Inherit time server attributes{/t}
{/render}
{else}
	<input disabled type='checkbox' name='option_disabled'>{t}Inherit time server attributes{/t}
{/if}
     </td>
    </tr>
    <tr>
     <td colspan="2" style='padding-left:14px;padding-top:5px;'><LABEL for="gotoNtpServerSelected">{t}NTP server{/t}</LABEL><br>
{render acl=$gotoNtpServerACL}
      <select name="gotoNtpServerSelected[]" id="gotoNtpServerSelected" multiple size=5 style="width:100%;"
            title="{t}Choose server to use for synchronizing time{/t}" {if $inheritTimeServer} disabled {/if}>
       {html_options options=$gotoNtpServer_select}
      </select>
{/render}
     <br>
{render acl=$gotoNtpServerACL}
      <select name="gotoNtpServers" id="gotoNtpServers" {if $inheritTimeServer} disabled {/if} >
       {html_options options=$gotoNtpServers}
      </select>
{/render}
{render acl=$gotoNtpServerACL}
        <input type="submit" name="addNtpServer" value="{msgPool type=addButton}"     id="addNtpServer"
         {if $inheritTimeServer} disabled {/if}>
{/render}
{render acl=$gotoNtpServerACL}
        <input type="submit" name="delNtpServer" value="{msgPool type=delButton}"  id="delNtpServer" 
         {if $inheritTimeServer} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>
{if $cn neq 'wdefault'}
<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>

{$netconfig}
{/if}
<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>

{if $fai_activated}
<h2><img class="center" alt="" align="middle" src="images/rocket.png"> {t}Action{/t}</h2>
<table width="100%">
<tr><td style="border-right:1px solid #AAA; width:50%">
<table summary="">
 <tr>
  <td>
{render acl=$FAIstateACL}
   <select size="1" name="saction" title="{t}Select action to execute for this terminal{/t}">
    <option>&nbsp;</option>
    {html_options options=$actions}
   </select>
{/render}
  </td>
  <td>
{if $currently_installing}
{render acl=r}
   <input type=submit name="action" value="{t}Execute{/t}">
{/render}
{else}
{render acl=$FAIstateACL}
   <input type=submit name="action" value="{t}Execute{/t}">
{/render}
{/if}
  </td>
 </tr>
</table>
{/if}
</td>
<td>
{if $member_of_ogroup}
   <input type='submit' name='inheritAll' value='{t}Inherit all values from group{/t}'>
{/if}
</td>
</tr></table>
<input type="hidden" name="workgeneric_posted" value="1">
 {if $cn eq 'wdefault'}
<!-- Place cursor -->
		<script language="JavaScript" type="text/javascript">
		  <!-- // First input field on page
			focus_field('l');
		  -->
		</script>
     {else}
		<script language="JavaScript" type="text/javascript">
		  <!-- // First input field on page
			focus_field('cn');
		  -->
		</script>
     {/if}

