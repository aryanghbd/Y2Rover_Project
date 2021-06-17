<h1 align="center">MongoDB Charts Embedding SDK</h1>

<div align="center">

Programmatically **embed** and **control** MongoDB Charts in your application.

[![npm](https://img.shields.io/npm/v/@mongodb-js/charts-embed-dom.svg)](https://www.npmjs.com/package/@mongodb-js/charts-embed-dom/)
[![npm](https://img.shields.io/npm/l/@mongodb-js/charts-embed-dom.svg)](https://www.npmjs.com/package/@mongodb-js/charts-embed-dom/)

<div style="max-width:600px;width:100%">

![demo of embedding](https://user-images.githubusercontent.com/19422770/79284750-efe86800-7efe-11ea-9ed0-9813e1a0b6d6.gif)
[Explore this example yourself!](https://codesandbox.io/s/github/mongodb-js/charts-embed-sdk/tree/master/examples/unauthenticated)

</div>

</div>

## Example Usage

```js
import ChartsEmbedSDK from '@mongodb-js/charts-embed-dom';

const sdk = new ChartsEmbedSDK({
  baseUrl: 'https://charts.mongodb.com/charts-charts-fixture-tenant-zdvkh',
});
const chart = sdk.createChart({
  chartId: '48043c78-f1d9-42ab-a2e1-f2d3c088f864',
});

// render the chart into a container
chart
  .render(document.getElementById('chart'))
  .catch(() => window.alert('Chart failed to initialise'));

// refresh the chart whenenver #refreshButton is clicked
document
  .getElementById('refreshButton')
  .addEventListener('click', () => chart.refresh());
```

## Getting Started

### Installation

[![module formats: umd, cjs, and esm](https://img.shields.io/badge/module%20formats-umd%2c%20cjs%2c%20esm-green.svg?style=flat)](https://unpkg.com/@mongodb-js/charts-embed-dom/dist/)

1. Install the `@mongodb-js/charts-embed-dom` package

```bash
# yarn
yarn add @mongodb-js/charts-embed-dom

# npm
npm install @mongodb-js/charts-embed-dom --save
```

2. Use the package

```js
import ChartsEmbedSDK from '@mongodb-js/charts-embed-dom';
```

3. Profit 📈

### Distribution Bundle

A [universal module definition](https://github.com/umdjs/umd) bundle is also published on npm under the /dist folder for consumption.

You can use the UMD to run `@mongodb-js/charts-embed-sdk` directly in the browser.

```html
<!-- library  -->
<script src="https://unpkg.com/@mongodb-js/charts-embed-dom"></script>

<script>
  const ChartsEmbedSDK = window.ChartsEmbedSDK;

  const sdk = new ChartsEmbedSDK({ ... });
  const chart = sdk.createChart({ ... });

  chart.render(document.getElementById('chart'));
</script>
```

## FAQs

### I'm using Rollup, what configuration options do I need to set?

To use the Embedding SDK with Rollup, you'll need to have a `rollup.config.js` that looks like:

```
import resolve from '@rollup/plugin-node-resolve';
import cjs from '@rollup/plugin-commonjs';

export default {
  input: 'index.js',
  output: {
    file: 'bundle.js',
    format: 'cjs'
  },
  plugins: [ cjs(), resolve({ browser: true, preferBuiltins: false }) ]
};
```

## API

### ChartsEmbedSDK

The default export of `@mongodb-js/charts-embed-dom`.

#### `constructor(options: EmbedChartOptions): ChartsEmbedSDK`

Creates an SDK object that can create Chart instances. Accepts an `object` that contains any default options to set for all Charts created using this SDK instance.

#### `createChart(options: EmbedChartOptions): Chart`

Creates an instance of a Chart that can be used to embed and control the MongoDB Chart specified by `chartId`.

### EmbedChartOptions

These options configure the behaviour of a Chart when it is first embedded. After this, you can control the configuration of the Chart by calling methods on its handle.

| name            | type              | description                                                                                                                                                                                                                                                                                                               |
| --------------- | ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| baseUrl         | string            | The base url for your MongoDB Charts instance, it should look something like: `https://charts.mongodb.com/charts-example-url-zdvkh`.                                                                                                                                                                                      |
| chartId         | string            | The chart id for the embedded chart. This can be found in the `Embed Chart` dialog when viewing a chart on a Dashboard.                                                                                                                                                                                                   |
| width           | string \| number  | The width of the embedded chart. If no width is provided, it will default to stretching to the width of it's container. If a value is provided without units, it will be assumed to be pixels (px).                                                                                                                       |
| height          | string \| number  | The height of the embedded chart. If no height is provided, it will default to stretching to the height of it's container. If a value is provided without units, it will be assumed to be pixels (px).                                                                                                                    |
| autoRefresh     | boolean           | Specifies whether the chart automatically refreshes. If omitted, charts do not automatically refresh. Use this option with the maxDataAge option to configure how often the chart refreshes.                                                                                                                              |
| maxDataAge      | number            | Specifies the maximum age of data to load from the cache when loading or refreshing the chart. If omitted, MongoDB Charts renders the chart with data from the cache if the data is less than one hour old.                                                                                                               |
| background      | string            | A [background color](https://developer.mozilla.org/en-US/docs/Web/CSS/background-color) for the embedded chart e.g. 'transparent'. If no background is provided it will set a default based on the theme.                                                                                                                 |
| filter          | object            | A filter to apply to the embedded chart. This expects an object that contains a valid [query operators](https://docs.mongodb.com/manual/reference/operator/query/#query-selectors). Any fields referenced in this filter are expected to be whitelisted in the 'Embed Chart' dialog for each Chart you wish to filter on. |
| getUserToken    | function          | A function that shoud return a valid JWT token to use for authenticating the render request.                                                                                                                                                                                                                              |
| theme           | 'light' \| 'dark' | The color scheme to apply to the chart. If the theme is set to 'dark', you will need to ensure that your background color has appropriate contrast as by default a chart's background is transparent.                                                                                                                     |
| showAttribution | boolean           | Whether to show the MongoDB attribution logo on the embedded chart. By default, this is set to `true`.                                                                                                                                                                                                                    |

### Chart

The Chart instance returned by `ChartsEmbedSDK.createChart({ ... })`.

#### `render(container: HTMLElement): Promise<void>`

Renders a chart into the specified container and should only be invoked once. It returns a Promise that will resolve once the chart is 'ready' to accepts commands (like `setFilter`, `refresh`).

#### `refresh(): Promise<void>`

Triggers a refresh of the chart, if it is embedded.

#### `setAutoRefresh(bool: boolean): Promise<void>`

Enable or disable chart autorefresh. Auto refresh is disabled by default.

#### `isAutoRefresh(): Promise<boolean>`

Returns whether auto refreshing is enabled or not.

#### `setMaxDataAge(seconds: number): Promise<void>`

Sets a duration (in seconds) for how old a chart is allowed to be before it is considered expired.

#### `getMaxDataAge(): Promise<number>`

Returns the duration (in seconds) that has to pass before a chart is considered stale.

#### `setFilter(filter: object): Promise<void>`

Applies a filter to the embedded chart. This expects an object that contains valid [query operators](https://docs.mongodb.com/manual/reference/operator/query/#query-selectors). Any fields referenced in this filter are expected to be whitelisted in the 'Embed Chart' dialog. An empty document `{}` is equivilent to no filter.

#### `getFilter(): Promise<object>`

Returns the current filter applied to the embedded chart.

#### `setTheme(theme: 'dark' | 'light'): Promise<void>`

Sets the current theme of the embedded chart. When setting the theme to 'dark', you will need to ensure that your background color has appropriate contrast as by default a chart's background is transparent.

#### `getTheme(): Promise<string>`

Returns the current theme applied to the embedded chart.

### getRealmUserToken(client: StitchClient): string

A helper function to use the Realm authentication provider.
Returns a value to pass to the `getUserToken` prop via a function.
