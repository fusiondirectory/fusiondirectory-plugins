<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span>{$section}</span></legend>
  <div>
  <table>
    {foreach from=$attributes item=attribute key=id}
      <tr>
        <td title="{$attribute.description}"><label for="{$attribute.htmlid}">{eval var=$attribute.label}</label></td>
        <td>{eval var=$attribute.input}</td>
        <td><input type="image" src="geticon.php?context=actions&icon=edit-delete&size=16" name="delOption_{$attribute.htmlid}" id="delOption_{$attribute.htmlid}" title="remove {eval var=$attribute.label}"/></td>
      </tr>
    {/foreach}
  </table>
  </div>
</fieldset>
