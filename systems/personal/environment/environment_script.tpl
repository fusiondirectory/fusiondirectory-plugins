<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span>{$section}</span></legend>
  <div>
  {$scripts=array('gotoLogonScript','gotoLogoffScript')}
  {foreach from=$scripts item=name}
    {$mainAttr={$name|cat:'s'}}
    {$mainAttr_buttons={$mainAttr|cat:'_buttons'}}
    {$nameAttr={$name|cat:'Name'}}
    {$contentAttr={$name|cat:'Content'}}
    {$descAttr={$name|cat:'Description'}}
    <table>
      <tr>
        <th title="{$attributes.$mainAttr.description}" colspan="2">
          {eval var=$attributes.$mainAttr.label}
        </th>
      </tr>
      <tr>
        <td title="{$attributes.$mainAttr.description}" colspan="2">
          {eval var=$attributes.$mainAttr.input}
        </td>
      </tr>
      <tr>
        <td title="{$attributes.$nameAttr.description}">
          <label for="{$attributes.$nameAttr.htmlid}">
            {eval var=$attributes.$nameAttr.label}
          </label>
        </td>
        <td>{eval var=$attributes.$nameAttr.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.$contentAttr.description}">
          <label for="{$attributes.$contentAttr.htmlid}">
            {eval var=$attributes.$contentAttr.label}
          </label>
        </td>
        <td>{eval var=$attributes.$contentAttr.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.$descAttr.description}">
          <label for="{$attributes.$descAttr.htmlid}">
            {eval var=$attributes.$descAttr.label}
          </label>
        </td>
        <td>{eval var=$attributes.$descAttr.input}</td>
      </tr>
      <tr>
        <td title="{$attributes.$mainAttr_buttons.description}" colspan="2">
          {eval var=$attributes.$mainAttr_buttons.input}
        </td>
      </tr>
    </table>
  {/foreach}
  </div>
</fieldset>
