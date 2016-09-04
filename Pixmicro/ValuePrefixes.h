// This file is part of Pixmicro.
// 
// Copyright (C) 2015 Kurt Skauen <http://kavionic.com/>
// 
// Pixmicro is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Pixmicro is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Pixmicro. If not, see < http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#pragma once

class ValuePrefixes : public QObject
{
    Q_OBJECT

public:
    enum Prefixes_e
    {
        e_PrefixNanometer,
        e_PrefixMicrometer,
        e_PrefixMillimeter,
        e_PrefixCentimeter,
        e_PrefixMeter,
        e_PrefixKilometers,
        e_PrefixLastMetric = e_PrefixKilometers,
        e_PrefixMetricAuto,
        e_PrefixInches,
        e_PrefixFeets,
        e_PrefixMils,
        e_PrefixMiles,
        e_PrefixCount,
    };

    static ValuePrefixes& Get();

    ValuePrefixes()
    {
        QSettings settings;

        s_SelectedPrefix = Prefixes_e(settings.value("Measure/units", ValuePrefixes::e_PrefixMicrometer).toInt());
        s_CalibrationScale = settings.value("Measure/calibrationScale", 1.0).toDouble();
    }

    void SaveSettings()
    {
        QSettings settings;

        settings.setValue("Measure/units", s_SelectedPrefix);
        settings.setValue("Measure/calibrationScale", s_CalibrationScale);
    }

    Prefixes_e GetSelectedPrefix() { return s_SelectedPrefix; }
    void SetSelectedPrefix(Prefixes_e prefix)
    {
        s_SelectedPrefix = prefix;
        emit SignalSelectedPrefixChanged(prefix);
    }

    void SetCalibrationScale(double value)
    {
        if (value != s_CalibrationScale) {
            s_CalibrationScale = value;
            emit SignalCalibrationChanged(value);
        }
    }
    double GetCalibrationScale() { return s_CalibrationScale; }

    static double GetPrefixScale(Prefixes_e prefix)
    {
        switch (prefix)
        {
            case e_PrefixNanometer:  return 1.0e-9;
            case e_PrefixMicrometer: return 1.0e-6;
            case e_PrefixMillimeter: return 1.0e-3;
            case e_PrefixCentimeter: return 1.0e-2;
            case e_PrefixMeter:      return 1.0;
            case e_PrefixKilometers: return 1.0e3;
            case e_PrefixInches:     return 0.0254;
            case e_PrefixFeets:      return 0.3048;
            case e_PrefixMils:       return 25.4e-6;
            case e_PrefixMiles:      return 1609.344;
            default: return 1.0;
        }
    }
    static QString GetPrefixName(Prefixes_e prefix)
    {
        switch (prefix)
        {
            case e_PrefixNanometer:  return "nm";
            case e_PrefixMicrometer: return "um";
            case e_PrefixMillimeter: return "mm";
            case e_PrefixCentimeter: return "cm";
            case e_PrefixMeter:      return "m";
            case e_PrefixKilometers: return "km";
            case e_PrefixMetricAuto: return "Metric(auto)";
            case e_PrefixInches:     return "in";
            case e_PrefixFeets:      return "ft";
            case e_PrefixMils:       return "mils";
            case e_PrefixMiles:      return "mile";
            default: return "";
        }
    }
    static Prefixes_e FindBestPrefix(double value)
    {
        for (int i = e_PrefixLastMetric ; i >= 1 ; ++i)
        {
            Prefixes_e prefix = Prefixes_e(i);
            double result = value / GetPrefixScale(prefix);
            if (value >= 1.0) return prefix;
        }
        return Prefixes_e(0);
    }

    static double ScaleToCalibration(double relValue)
    {
        return relValue * Get().s_CalibrationScale;
    }
    static QString FormatValue(double value) {
        return QString::asprintf("%.3f", ScaleToCalibration(value) / GetPrefixScale(Get().s_SelectedPrefix)) + GetPrefixName(Get().s_SelectedPrefix);
    }
    static QString FormatArea(double value) {
        double area = ScaleToCalibration(sqrt(value)) / GetPrefixScale(Get().s_SelectedPrefix);
        area = area * area;
        return QString::asprintf("%.3f", area) + GetPrefixName(Get().s_SelectedPrefix) + "^2";
    }
    static QString FormatAngle(double value) {
        return QString::asprintf("%.2fdeg", value * 360.0 / (M_PI * 2.0));
    }


signals:

    void SignalSelectedPrefixChanged(Prefixes_e prefix);
    void SignalCalibrationChanged(double calibrationScale);

private:
    Prefixes_e s_SelectedPrefix;
    double     s_CalibrationScale;
};

