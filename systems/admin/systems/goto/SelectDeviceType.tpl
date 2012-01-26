<br>
<p class="seperator">
{t}This system has no system type configured. Please choose a system type for this object and an optional inheritance group. Press the 'continue' button to proceed.{/t}
<br>
<br>
</p>
<p class="seperator">
<br>
{if $dns_cnt == 1}
<b>{t}Please select a system type and an optional bundle of predefined settings to be inherited.{/t}</b>
{else}
<b>{t}Please select a system type and a bundle of predefined settings to be inherited.{/t}</b>
{/if}
<br>
<br>
</p>
<table summary="" style='width:100%'>
 <tr>
  <td style='width:49%'>
   <table summary="">
    <tr>
     <td> 
      {t}System type{/t}&nbsp;
	  <select name="SystemType" title="{t}System type{/t}" style="width:120px;"
			onChange="document.mainform.submit();">
       {html_options values=$SystemTypeKeys output=$SystemTypes selected=$SystemType}
      </select>
     </td>
    </tr>
   </table>
  </td>
  <td>
   <table summary="">
    <tr>
     <td> 
      {t}Choose an object group as template{/t}&nbsp;
	  <select name="ObjectGroup" title="{t}Object group{/t}" style="width:120px;">
		{if $dns_cnt == 1}
		<option value='none'>{t}none{/t}</option>	
		{/if}
       {html_options options=$ogroups selected=$ObjectGroup}
      </select>
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>
<p class="seperator">&nbsp;</p>
<div style="align: right;" align="right"><p>
	<input type="submit" name="systemTypeChosen" value="{t}Continue{/t}">
	<input type="submit" name="SystemTypeAborted" value="{msgPool type=cancelButton}">
</p></div>
