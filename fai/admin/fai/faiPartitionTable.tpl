<table width="100%" summary="">
	<tr>
		<td width="50%" valign="top">
				<h2><img class="center" alt="" src="plugins/fai/images/fai_small.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
				<table summary="" cellspacing="4">
					<tr>
						<td>
							<LABEL for="cn">
							{t}Name{/t}{$must}
							</LABEL>
						</td>
						<td>
{render acl=$cnACL}
							<input type='text' value="{$cn}" size="45" maxlength="80" id='cn' disabled >
{/render}
						</td>
					</tr>
					<tr>
						<td>
							<LABEL for="description">
							{t}Description{/t}
							</LABEL>
						</td>
						<td>
{render acl=$descriptionACL}
							<input type='text' value="{$description}" size="45" maxlength="80" name="description" id="description">
{/render}
						</td>
					</tr>
				</table>
                                <p class="seperator">&nbsp;</p>
                                <p>
                                <input type="checkbox" name="mode" value="1" {$mode} {$lockmode} onClick="changeState('AddRaid'); changeState('AddVolgroup');"> {t}Use 'setup-storage' to partition the disk{/t}                                      </p>
		</td>
		<td style="border-left: 1px solid rgb(160, 160, 160);">
		   &nbsp;
	 	</td>
		<td>
				<h2><img class="center" alt="" src="plugins/fai/images/fai_partitionTable.png" align="middle" title="{t}Objects{/t}">&nbsp;
					<LABEL for="SubObject">
						{t}Discs{/t}
					</LABEL>
				</h2>
                                {$Entry_divlist}
{if $sub_object_is_addable}
                                <input type="submit" name="AddDisk" value="{t}Add disk{/t}" title="{t}Add disk{/t}">
                                <input type="submit" id="AddRaid" name="AddRaid" value="{t}Add RAID{/t}" title="{t}Add RAID{/t}" {$storage_mode} {$addraid}>
                                <input type="submit" id="AddVolgroup" name="AddVolgroup" value="{t}Add volume group{/t}" title="{t}Add volume group{/t}" {$storage_mode}>
{else}
                                <input type="submit" name="AddDisk" value="{t}Add disk{/t}" title="{t}Add disk{/t}" disabled>
                                <input type="submit" name="AddRaid" value="{t}Add RAID{/t}" title="{t}Add RAID{/t}" disabled>
                                <input type="submit" name="AddVolgroup" value="{t}Add volume group{/t}" title="{t}Add volume group{/t}" disabled>
{/if}

		</td>
	</tr>
</table>
<input type='hidden' name='FAIpartitionTablePosted' value='1'>
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>

