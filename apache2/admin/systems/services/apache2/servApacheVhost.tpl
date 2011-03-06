<h2>{t}Apache VHosts{/t}</h2>
<table summary="" width="100%">
<tr>
	<td style="width:100%;vertical-align:top;">
		{$VhostList}

		{render acl=$VirtualHostACL}
		<input type="submit" name="AddVhost" value="{t}Add{/t}">
		{/render}
	</td>
</tr>
</table>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  document.mainform.AddVhost.focus();
  -->
</script>
<input type="hidden" name="servapache" value="1">

<p class="seperator">&nbsp;</p>
<br>
<div style="width:100%; text-align:right;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>

