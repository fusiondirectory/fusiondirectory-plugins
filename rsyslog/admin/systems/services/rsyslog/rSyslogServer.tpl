<h2><img class="center" alt="" align="middle" src="images/rightarrow.png" /> {t}Syslog logging{/t}</h2>

<br>
<input type="checkbox" name="use_database" value="1" {if $use_database} checked {/if}
  onChange="changeState('gosaLogDB'); changeState('goLogAdmin');changeState('goLogPassword');"
  class="center">
<b>{t}Server provides a syslog mysql database{/t}</b>
<table summary="">
    <tr>
     <td>{t}Database{/t}{$must}</td>
     <td>
{render acl=$gosaLogDBACL}
 	<input name="gosaLogDB" id="gosaLogDB" size=30 maxlength=60 value="{$gosaLogDB}"
    {if !$use_database} disabled {/if}>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Database user{/t}{$must}</td>
     <td>
{render acl=$goLogAdminACL}
	<input name="goLogAdmin" id="goLogAdmin" size=30 maxlength=60 value="{$goLogAdmin}"
    {if !$use_database} disabled {/if}>
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Password{/t}{$must}</td>
     <td>
{render acl=$goLogPasswordACL}
 	<input type="password" name="goLogPassword" id="goLogPassword" size=30 maxlength=60 value="{$goLogPassword}"
    {if !$use_database} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="rSyslogServerPosted" value="1">
