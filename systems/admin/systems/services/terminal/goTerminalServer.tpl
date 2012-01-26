<table summary="">
    <tr>
     <td colspan=2>
		<h2><img class="center" alt="" align="middle" src="images/rightarrow.png" /> {t}Terminal service{/t}</h2>
{render acl=$goXdmcpIsEnabledACL}
      <input type="checkbox" value="true" name="goXdmcpIsEnabled" id="Temporariid" {if $goXdmcpIsEnabled} checked {/if}>
{/render}
	{t}Temporary disable login{/t}
     </td>
    </tr>
    <tr>
     <td colspan=2>
		<h2>{t}Supported session types{/t}</h2>
			{foreach from=$gotoSessionTypes item=item key=key}
				<input class="center" type="checkbox" name="gotoSessionType_{$key}" 
					{if $item} checked {/if}>&nbsp;{$key} <br>
			{/foreach}
     </td>
    </tr>
   </table>

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="goTerminalServerPosted" value="1">
