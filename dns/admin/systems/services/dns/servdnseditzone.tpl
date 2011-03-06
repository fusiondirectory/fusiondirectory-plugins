<h2>{t}Generic{/t}</h2>
<table summary="" width="100%">
	<tr>
		<td style="width:50%;vertical-align:top;border-right:1px	solid	#b0b0b0;">
			<table summary="">
				<tr>
					<td style='vertical-align:top'>{t}Zone name{/t}{$must}
					</td>
					<td>
{render acl=$zoneNameACL}					
						<input type="text" name="zoneName" value="{$zoneName}" {if $NotNew || $Zone_is_used} disabled {/if}>
{/render}
					</td>
				</tr>
				<tr>
					<td style='vertical-align:top'>{t}Network address{/t}{$must}
					</td>
					<td>
{render acl=$ReverseZoneACL}					
						<input type="text" name="ReverseZone" value="{$ReverseZone}" {if $NotNew || $Zone_is_used} disabled {/if}>
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Netmask{/t}
					</td>
					<td>
{render acl=$NetworkClassACL}					
						<select name="NetworkClass" {if $NotNew || $Zone_is_used} disabled {/if}>
							{html_options options=$NetworkClasses selected=$NetworkClass}
						</select>
{/render}
					</td>
				</tr>
				{if $Zone_is_used}
				<tr>
					<td colspan="2">
						<i>{t}Zone is in use, network settings can't be modified.{/t}</i>
					</td>
				</tr>
				{/if}
			</table>
		</td>
		<td style="vertical-align:top;">
			<table summary="">
				<tr>
					<td>
						{t}Zone records{/t}
						<br>
						{if $AllowZoneEdit == false}
							<i>{t}Can't be edited because the zone wasn't saved right now.{/t}</i>
						{/if}
					</td>
					<td>
{render acl=$zoneEditorACL mode=read_active}
						<input type="submit" name="EditZoneEntries" value="{t}Edit{/t}" {if $AllowZoneEdit == false} disabled {/if}> 
{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<p class="seperator">&nbsp;</p>
<h2>{t}SOA record{/t}</h2>
<table summary="" width="100%">
	<tr>
		<td style="vertical-align:top;width:50%;border-right:1px	solid	#b0b0b0;">
			<table summary="">
				<tr>
					<td>{t}Primary dns server for this zone{/t}{$must}
					</td>
					<td>
{render acl=$sOAprimaryACL}					
						<input type="text" name="sOAprimary" value="{$sOAprimary}">
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Mail address{/t}{$must}
					</td>
					<td>
{render acl=$sOAmailACL}					
						<input type="text" name="sOAmail" value="{$sOAmail}">
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Serial number (automatically incremented){/t}{$must}
					</td>
					<td>
{render acl=$sOAserialACL}					
						<input type="text" name="sOAserial" value="{$sOAserial}">
{/render}
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:top;">
			<table summary="">
				<tr>
					<td>{t}Refresh{/t}{$must}
					</td>
					<td>
{render acl=$sOArefreshACL}					
						<input type="text" name="sOArefresh" value="{$sOArefresh}">
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Retry{/t}{$must}
					</td>
					<td>
{render acl=$sOAretryACL}					
						<input type="text" name="sOAretry" value="{$sOAretry}">
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Expire{/t}{$must}
					</td>
					<td>
{render acl=$sOAexpireACL}					
						<input type="text" name="sOAexpire" value="{$sOAexpire}">
{/render}
					</td>
				</tr>
				<tr>
					<td>{t}TTL{/t}{$must}
					</td>
					<td>
{render acl=$sOAttlACL}					
						<input type="text" name="sOAttl" value="{$sOAttl}">
{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>

<p class="seperator">&nbsp;</p>
<br>
<table summary="" width="100%">
	<tr>
		<td style="vertical-align:top;width:50%;border-right:1px	solid	#b0b0b0;">
			<h2>{t}MxRecords{/t}</h2>
			<table width="100%">	
				<tr>
					<td>
{render acl=$mXRecordACL}					
						{$Mxrecords}
{/render}
{render acl=$mXRecordACL}					
						<input type="text" 		name="StrMXRecord" value="">
{/render}
{render acl=$mXRecordACL}					
						<input type="submit" 	name="AddMXRecord" value="{msgPool type=addButton}">
{/render}
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:top;">
			<h2>{t}Global zone records{/t}</h2>
{render acl=$zoneRecordsACL}					
			  {$records}
{/render}
		</td>
	</tr>
</table>
<div style="text-algin:right;" align="right">
	<p>
		<input type="submit" name="SaveZoneChanges" value="{msgPool type=saveButton}">
		<input type="submit" name="CancelZoneChanges" value="{msgPool type=cancelButton}">
	</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('zoneName');
  -->
</script>
