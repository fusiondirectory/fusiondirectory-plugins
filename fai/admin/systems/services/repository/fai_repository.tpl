<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span><label for="{$attributes.FAIrepository.htmlid}">{$section}</label></span></legend>
  <div>
  <table>
    <tr>
      <td title="{$attributes.FAIrepository.description}" colspan="4">
        {eval var=$attributes.FAIrepository.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.distribution.description}">
        <label for="{$attributes.distribution.htmlid}">
          {eval var=$attributes.distribution.label}
        </label>
      </td>
      <td>{eval var=$attributes.distribution.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.mirrorUrl.description}">
        <label for="{$attributes.mirrorUrl.htmlid}">
          {eval var=$attributes.mirrorUrl.label}
        </label>
      </td>
      <td>{eval var=$attributes.mirrorUrl.input}</td>

      <td rowspan="3" title="{$attributes.mirrorArchs_inner.description}">
        <label for="{$attributes.mirrorArchs_inner.htmlid}">
          {eval var=$attributes.mirrorArchs_inner.label}
        </label><br/>
        {eval var=$attributes.mirrorArchs_inner.input}
      </td>
    </tr>
    <tr>
      <td title="{$attributes.mirrorParent.description}">
        <label for="{$attributes.mirrorParent.htmlid}">
          {eval var=$attributes.mirrorParent.label}
        </label>
      </td>
      <td>{eval var=$attributes.mirrorParent.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.mirrorRelease.description}">
        <label for="{$attributes.mirrorRelease.htmlid}">
          {eval var=$attributes.mirrorRelease.label}
        </label>
      </td>
      <td>{eval var=$attributes.mirrorRelease.input}</td>
    </tr>
    <tr>
      <td title="{$attributes.mirrorMode.description}">
        <label for="{$attributes.mirrorMode.htmlid}">
          {eval var=$attributes.mirrorMode.label}
        </label>
      </td>
      <td>{eval var=$attributes.mirrorMode.input}</td>

      {if isset($attributes.mirrorSections_inner)}
      <td rowspan="3" title="{$attributes.mirrorSections_inner.description}">
        <label for="{$attributes.mirrorSections_inner.htmlid}">
          {eval var=$attributes.mirrorSections_inner.label}
        </label><br/>
        {eval var=$attributes.mirrorSections_inner.input}
      </td>
      {/if}
    </tr>
    <tr>
      <td title="{$attributes.mirrorLocal.description}">
        <label for="{$attributes.mirrorLocal.htmlid}">
          {eval var=$attributes.mirrorLocal.label}
        </label>
      </td>
      <td>{eval var=$attributes.mirrorLocal.input}</td>
    </tr>
    <tr>
      {if isset($attributes.pathMask)}
      <td title="{$attributes.pathMask.description}">
        <label for="{$attributes.pathMask.htmlid}">
          {eval var=$attributes.pathMask.label}
        </label>
      </td>
      <td>{eval var=$attributes.pathMask.input}</td>
      {/if}
    </tr>
    <tr>
      <td title="{$attributes.FAIrepository_buttons.description}" colspan="3">
        {eval var=$attributes.FAIrepository_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
