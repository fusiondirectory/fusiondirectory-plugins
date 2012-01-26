<h2><img alt="P" class="center" src="plugins/systems/images/select_printer.png" align="middle">&nbsp;{t}Printer driver{/t}</h2>
{if !$path_valid}
<p>
	<b>{msgPool type=invalidConfigurationAttribute param=ppdPath}</b>
</p>
{else}
	<table summary="" width="100%">
		<tr>
			<td width="50%" style="vertical-align:top">
				{t}Model{/t}: <i>{$ppdString}</i>&nbsp;
				{render acl=$acl}
				<input type="submit" name="SelectPPD" value="{t}Select{/t}">
				{/render}
			</td>
			<td style="border-left: 1px solid rgb(160, 160, 160);padding-left:10px;">
				{t}New driver{/t}&nbsp;
				{render acl=$acl}
				<input type="file" value="" name="NewPPDFile">
				{/render}
				{render acl=$acl}
				<input type="submit" name="SubmitNewPPDFile" value="{t}Upload{/t}">
				{/render}
			</td>
		</tr>
	</table>
	{if $showOptions eq 1}
	<p class="seperator">&nbsp;</p>
	<h2><img alt="L" class="center" src="images/lists/on.png" align="middle">&nbsp;{t}Options{/t}</h2>
	{$properties}
	{/if}
{/if}
<p class="plugbottom">
	<input type="hidden" name="PPDDisSubmitted" value="1">
	{if $path_valid}
	{render acl=$acl}
	<input type="submit" name="SavePPD" value="{msgPool type=applyButton}">
	{/render}
	{/if}
	<input type="submit" name="ClosePPD" value="{msgPool type=cancelButton}">
</p>
