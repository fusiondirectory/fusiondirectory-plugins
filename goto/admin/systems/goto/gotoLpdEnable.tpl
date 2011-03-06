{if !$is_account}

<table style='width:100%;'>
	<tr>
		<td style='width:55%;'>
			<table>
				<tr>
					<td colspan="2">
						{render acl=$acl}
						<input class="center" type='checkbox' onChange="document.mainform.submit();" 
							{if $is_account} checked {/if}
							name='gotoLpdEnable_enabled'>&nbsp;{t}Enable printer settings{/t}</td>
						{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
{else}
<table style='width:100%;'>
	<tr>
		<td style='width:55%;'>
			<table>
<!--
				<tr>
					<td>{t}Device{/t}&nbsp;#</td>
					<td>
						<select name='DevID' onChange="document.mainform.submit();">
							{foreach from=$data key=key item=item}
	<option {if $key == $DevID} selected {/if} value='{$key}'>{$key+1}: {$item.s_Type} {$item.s_Device}</option>
							{/foreach}
						</select>
						{if $data_cnt >= 3}
						<input type="button" disabled name="dummy1" value="{msgPool type=addButton}">
						{else}
						<input type="submit" name="add_printer" value="{msgPool type=addButton}">
						{/if}
						<input type="submit" name="del_printer" value="{msgPool type=delButton}">
					</td>
				</tr>
-->
				<tr>
					<td colspan="2">
						{render acl=$acl}
						<input class="center" type='checkbox' onChange="document.mainform.submit();" 
							{if $is_account} checked {/if}
							name='gotoLpdEnable_enabled'>&nbsp;{t}Enable printer settings{/t}</td>
						{/render}
					</TD>
				</tr>
				<tr>
					<td>{t}Type{/t}</td>
					<td>	
						{render acl=$acl}
						<select name='s_Type'  onChange="document.mainform.submit();">
							{html_options options=$a_Types selected=$s_Type}
						</select>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Device{/t}</td>
					<td>	
						{render acl=$acl}
						<input type='text' name='s_Device' value='{$s_Device}'>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Port{/t}</td>
					<td>
						{render acl=$acl}
						<input type='text' name='i_Port' value='{$i_Port}'>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Options{/t}</td>
					<td>
						{render acl=$acl}
						<input type='text' name='s_Options' value='{$s_Options}'>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Write only{/t}</td>
					<td>
						{render acl=$acl}
						<input {if $s_WriteOnly == "Y"} checked {/if} type='checkbox' name='s_WriteOnly' value='Y' >
						{/render}
					</td>
				</tr>
			</table>
		</td>
		<td>
{if $s_Type == "S"}
			<table>
				<tr>
					<td>{t}Bit rate{/t}</td>
					<td>
						{render acl=$acl}
						<select name='s_Speed'>
							{html_options options=$a_Speeds selected=$s_Speed}
						</select>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Flow control{/t}</td>
					<td>
						{render acl=$acl}
						<select name='s_FlowControl'>
							{html_options options=$a_FlowControl selected=$s_FlowControl}
						</select>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Parity{/t}</td>
					<td>
						{render acl=$acl}
						<select name='s_Parity'>
							{html_options options=$a_Parities selected=$s_Parity}
						</select>
						{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Bits{/t}</td>
					<td>
						{render acl=$acl}
						<select name='i_Bit'>
							{html_options options=$a_Bits selected=$i_Bit}
						</select>
						{/render}
					</td>
				</tr>
			</table>
{/if}
		</td>
	</tr>
</table>
{/if}
<input type='hidden' name="gotoLpdEnable_entry_posted" value="1">
