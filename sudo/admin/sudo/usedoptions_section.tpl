<fieldset id="{$sectionId}" class="plugin_section">
  <legend>{$section}</legend>
  <table>
    {foreach from=$attributes item=attribute key=id}
      <tr>
        <td title="{$attribute.description}"><label for="{$id}">{eval var=$attribute.label}</label></td>
        <td>{eval var=$attribute.input}</td>
        <td><input type="image" src="images/lists/trash.png" name="delOption_{$id}" id="delOption_{$id}" title="remove {eval var=$attribute.label}"/></td>
      </tr>
    {/foreach}
  </table>
</fieldset>
