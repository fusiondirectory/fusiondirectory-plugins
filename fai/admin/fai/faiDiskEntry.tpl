<input type="hidden" name="TableEntryFrameSubmitted" value="1">
<h2><img class="center" alt="" src="plugins/fai/images/fai_partitionTable.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Device{/t}</h2>
<table style='width:100%' summary="">
	<tr>
		<td style='width:50%;border-right:1px solid #909090;'>

      <table>
        <tr>
          <td>
            <LABEL for="DISKcn">{t}Name{/t}</LABEL>{$must}&nbsp;
          </td>
          <td>
{render acl=$DISKcnACL}
			      <input type='text' value="{$DISKcn}" size="45" maxlength="80" name="DISKcn" id="DISKcn">
{/render}
      		</td>
        </tr>
        <tr>
		      <td>
            <LABEL for="fstabkey">{t}fstab key{/t}</LABEL>
          </td>
          <td>
{render acl=$DISKFAIdiskOptionACL}
            <select name="fstabkey" size="1">
               {html_options options=$fstabkeys selected=$fstabkey}
            </select>
{/render}
		      </td>
        </tr>
      </table>

    </td>
		<td>

      <table>
        <tr>
          <td>
            <LABEL for="DISKdescription">{t}Description{/t}</LABEL>&nbsp;
          </td>
          <td>
{render acl=$DISKdescriptionACL}
			      <input value="{$DISKdescription}" size="45" maxlength="80" 
              name="DISKdescription" id="DISKdescription">
{/render}
		      </td>
        </tr>
        <tr>
		      <td>
            <LABEL for="disklabel">{t}Disk label{/t}</LABEL>
          </td>
          <td>
{render acl=$DISKFAIdiskOptionACL}
            <select name="disklabel" size="1">
               {html_options options=$disklabels selected=$disklabel}
            </select>
{/render}
		      </td>
        </tr>
      </table>

    </td>
	</tr>
</table>

{if $FAIdiskType == "lvm"}

<p class="seperator">&nbsp;</p>
<table width="100%">
  <tr>
    <td>
      <h2>{t}Combined physical partitions{/t}</h2>

      <select style='font-family: monospace; width: 100%;' 
        name='physicalPartition[]' size=5 multiple> 
        {html_options options=$plist}
      </select>
      <br>
      <select name='lvmPartitionAdd' style='width:240px;'>
        {html_options options=$physicalPartitionList}
      </select>
      <input type="submit" name='addLvmPartition' value="{msgPool type="addButton"}">&nbsp;
      <input type="submit" name='delLvmPartition' value="{msgPool type="delButton"}">&nbsp;
    </td>
  </tr>
</table>

{/if}
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

