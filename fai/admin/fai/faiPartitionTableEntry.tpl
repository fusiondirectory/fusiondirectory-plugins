<input type="hidden" name="TableEntryFrameSubmitted" value="1">
<h2><img class="center" alt="" src="plugins/fai/images/fai_partitionTable.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Device{/t}</h2>
<table style='width:100%' summary="">
	<tr>
		<td style='width:50%;border-right:1px solid #909090;'><LABEL for="DISKcn">
			{t}Name{/t}
			</LABEL>{$must}&nbsp;
{render acl=$DISKcnACL}
			<input type='text' value="{$DISKcn}" size="45" maxlength="80" name="DISKcn" id="DISKcn">
{/render}
		</td>
		<td><LABEL for="DISKdescription">
			&nbsp;{t}Description{/t}
			</LABEL>&nbsp;
{render acl=$DISKdescriptionACL}
			<input type='text' value="{$DISKdescription}" size="45" maxlength="80" name="DISKdescription" id="DISKdescription">
{/render}
		</td>
	</tr>
</table>
<br>
<p class="seperator">&nbsp;</p>
<br>
<h2><img class="center" alt="" src="images/lists/paste.png" align="middle" title="{t}Partition entries{/t}">&nbsp;{t}Partition entries{/t}</h2>
{$setup}
<br>
{if !$freeze}
	{if $sub_object_is_createable}
		<input type="submit" name="AddPartition" value="{t}Add partition{/t}">
	{else}
		<input type="submit" name="restricted" value="{t}Add partition{/t}" disabled>
	{/if}
{/if}
<br>	
<br>
<p class="seperator">&nbsp;</p>
<br>
<div style="align:right;" align="right">
{if !$freeze}
	<input type="submit" name="SaveDisk" value="{msgPool type=saveButton}">
{/if}
<input type="submit" name="CancelDisk" value="{msgPool type=cancelButton}" >
</div>
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('DISK_cn');
  -->
</script>

